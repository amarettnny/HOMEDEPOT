// server.cpp

#include <pthread.h>
#include <iostream>
#include <sstream>
#include <set>
#include <vector>
#include <cassert>

#include "csapp.h"           // open_listenfd, Accept, Pthread_create
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Per-client state
////////////////////////////////////////////////////////////////////////
namespace {

struct Client_thread {
    Server*     server;
    Connection* conn;
    User*       user;
    Room*       room;

    Client_thread(Server* s, Connection* c)
      : server(s), conn(c), user(nullptr), room(nullptr)
    {}

    ~Client_thread() {
        delete conn;
        delete user;
    }
};

////////////////////////////////////////////////////////////////////////
// Sender loop: support /join, /leave, sendall, quit
////////////////////////////////////////////////////////////////////////
static void chat_with_sender(Client_thread* c) {
    Connection* conn   = c->conn;
    Server*     server = c->server;
    User*       user   = c->user;
    Room*       room   = c->room;

    while (true) {
        Message in;
        if (!conn->receive(in)) {
            if (conn->get_last_result() == Connection::INVALID_MSG) {
                conn->send(Message(TAG_ERR,
                    "Invalid message, please try again"));
            }
            return;
        }

        // Trim trailing newline
        if (!in.data.empty() && in.data.back() == '\n') {
            in.data.pop_back();
        }

        // Handle commands
        if (in.tag == TAG_QUIT) {
            conn->send(Message(TAG_OK, "bye"));
            if (room) room->remove_member(user);
            return;
        }
        else if (in.tag == TAG_JOIN) {
            if (room) {
                room->remove_member(user);
            }
            Room* newroom = server->find_or_create_room(in.data);
            newroom->add_member(user);
            room = newroom;
            c->room = newroom;
            conn->send(Message(TAG_OK,
                "Successfully joined " + in.data));
        }
        else if (in.tag == TAG_LEAVE) {
            if (!room) {
                conn->send(Message(TAG_ERR,
                    "You are not in a room"));
            } else {
                room->remove_member(user);
                room = nullptr;
                c->room = nullptr;
                conn->send(Message(TAG_OK,
                    "Successfully left room"));
            }
        }
        else if (in.tag == TAG_SENDALL) {
            if (!room) {
                conn->send(Message(TAG_ERR,
                    "Join a room before sending"));
            } else {
                room->broadcast_message(user->username, in.data);
                conn->send(Message(TAG_OK, "Message sent"));
            }
        }
        else {
            conn->send(Message(TAG_ERR,
                "Unknown command; use /join, /leave, /sendall, or /quit"));
        }
    }
}

////////////////////////////////////////////////////////////////////////
// Receiver loop: deliver TAG_DELIVERY messages
////////////////////////////////////////////////////////////////////////
static void chat_with_receiver(Client_thread* c) {
    Connection* conn = c->conn;
    User*       user = c->user;
    Room*       room = c->room;

    while (true) {
        Message* msg = user->mqueue.dequeue();
        if (msg) {
            if (!conn->send(*msg)) {
                delete msg;
                if (room) room->remove_member(user);
                return;
            }
            delete msg;
        }
    }
}

////////////////////////////////////////////////////////////////////////
// Thread entrypoint: login, join, then dispatch
////////////////////////////////////////////////////////////////////////
static void* worker(void* arg) {
    pthread_detach(pthread_self());
    Client_thread* client = static_cast<Client_thread*>(arg);

    // Login
    Message login;
    if (!client->conn->receive(login)) {
        client->conn->send(Message(TAG_ERR,
            "Couldn't receive login message"));
        delete client;
        return nullptr;
    }
    if (login.tag != TAG_SLOGIN && login.tag != TAG_RLOGIN) {
        client->conn->send(Message(TAG_ERR,
            "Invalid login; use slogin or rlogin"));
        delete client;
        return nullptr;
    }
    client->conn->send(Message(TAG_OK, "Logged in"));

    // Strip newline from username
    std::string uname = login.data;
    if (!uname.empty() && uname.back() == '\n') {
        uname.pop_back();
    }
    client->user = new User(uname);

    // Join
    Message join;
    if (!client->conn->receive(join)) {
        client->conn->send(Message(TAG_ERR,
            "Couldn't receive join message"));
        delete client;
        return nullptr;
    }
    // quit before ever joining
    if (join.tag == TAG_QUIT) {
      client->conn->send(Message(TAG_OK, "bye"));
      delete client;
      return nullptr;
    }
    if (join.tag != TAG_JOIN) {
        client->conn->send(Message(TAG_ERR,
            "Must join a room first"));
        delete client;
        return nullptr;
    }
    // Trim newline from room name
    std::string roomname = join.data;
    if (!roomname.empty() && roomname.back() == '\n') {
        roomname.pop_back();
    }
    Room* room = client->server->find_or_create_room(roomname);
    room->add_member(client->user);
    client->room = room;
    client->conn->send(Message(TAG_OK,
        "Successfully joined " + roomname));

    // Chat with sender/receiver based on tag
    if (login.tag == TAG_SLOGIN) {
        chat_with_sender(client);
    } else {
        chat_with_receiver(client);
    }

    // Cleanup
    if (client->room) {
        client->room->remove_member(client->user);
    }
    delete client;
    return nullptr;
}

} 

////////////////////////////////////////////////////////////////////////
// Server methods
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port), m_ssock(-1)
{
    pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
    pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
    std::string ps = std::to_string(m_port);
    m_ssock = open_listenfd(ps.c_str());
    return m_ssock >= 0;
}

void Server::handle_client_requests() {
    while (true) {
        int fd = Accept(m_ssock, nullptr, nullptr);
        if (fd < 0) {
            std::cerr << "Accept failed\n";
            return;
        }
        Connection* conn = new Connection(fd);
        Client_thread* client = new Client_thread(this, conn);
        pthread_t tid;
        Pthread_create(&tid, nullptr, worker, client);
    }
}

Room* Server::find_or_create_room(const std::string& name) {
    Guard lock(m_lock);
    auto it = m_rooms.find(name);
    if (it != m_rooms.end()) {
        return it->second;
    }
    Room* r = new Room(name);
    m_rooms[name] = r;
    return r;
}
