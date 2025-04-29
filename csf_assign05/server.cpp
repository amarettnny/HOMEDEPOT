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


namespace {

////////////////////////////////////////////////////////////////////////
// Server implementation data types 
////////////////////////////////////////////////////////////////////////
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
// Client thread functions
////////////////////////////////////////////////////////////////////////
static void chat_with_sender(Client_thread* c) {
    // loop handling a sender client
    Connection* conn   = c->conn;
    Server*     server = c->server;
    User*       user   = c->user;
    Room*       room   = c->room;

    while (true) {
        Message in;
        if (!conn->receive(in)) { //receive next client message
            if (conn->get_last_result() == Connection::INVALID_MSG) {
                conn->send(Message(TAG_ERR, "Invalid message"));
            }
            return;
        }

        // Trim any trailing line terminator from messgae
        if (!in.data.empty() && in.data.back() == '\n') {
            in.data.pop_back();
        }

        // Handle commands based on tags
        if (in.tag == TAG_QUIT) {
            conn->send(Message(TAG_OK, "bye"));
            if (room) room->remove_member(user); //remove if currently joined
            return;
        }
        else if (in.tag == TAG_JOIN) {
            if (room) { //leave old room if any
                room->remove_member(user);
            }
            Room* newroom = server->find_or_create_room(in.data);
            newroom->add_member(user); //join new room
            room = newroom;
            c->room = newroom;
            conn->send(Message(TAG_OK, "Successfully joined " + in.data));
        }
        else if (in.tag == TAG_LEAVE) {
            if (!room) { //if currently not in a room
                conn->send(Message(TAG_ERR, "Cannot leave if not in a room"));
            } else {
                room->remove_member(user); //leave the room
                room = nullptr;
                c->room = nullptr;
                conn->send(Message(TAG_OK, "Successfully left"));
            }
        }
        else if (in.tag == TAG_SENDALL) { // Broadcasting
            if (!room) {
                conn->send(Message(TAG_ERR, "Cannot broadcast if not in a room"));
            } else {
                room->broadcast_message(user->username, in.data);
                conn->send(Message(TAG_OK, "Message sent"));
            }
        }
        else { //Invald tags, report error
            conn->send(Message(TAG_ERR, "Invalid command"));
        }
    }
}

// Receiver loop delivers queued messages to a receiver
static void chat_with_receiver(Client_thread* c) {
    Connection* conn = c->conn;
    User*       user = c->user;
    Room*       room = c->room;

    while (true) {
        // block until message is available
        Message* msg = user->mqueue.dequeue();
        if (msg) {
            if (!conn->send(*msg)) { // if fail
                delete msg;
                if (room) room->remove_member(user); //clean up and exit
                return;
            }
            delete msg;
        }
    }
}

static void* worker(void* arg) {
    pthread_detach(pthread_self());
    Client_thread* client = static_cast<Client_thread*>(arg);

    // Login
    Message login;
    if (!client->conn->receive(login)) {
        client->conn->send(Message(TAG_ERR, "Couldn't receive login message"));
        delete client;
        return nullptr;
    }
    if (login.tag != TAG_SLOGIN && login.tag != TAG_RLOGIN) { //if invalid login
        client->conn->send(Message(TAG_ERR, "Invalid login"));
        delete client;
        return nullptr;
    }
    // Succuessful login
    client->conn->send(Message(TAG_OK, "Logged in"));

    // newline from username
    std::string uname = login.data;
    if (!uname.empty() && uname.back() == '\n') {
        uname.pop_back();
    }
    client->user = new User(uname); //create user

    // Join
    Message join;
    if (!client->conn->receive(join)) {
        client->conn->send(Message(TAG_ERR, "Couldn't receive join message"));
        delete client;
        return nullptr;
    }
    // allow quitting before any join
    if (join.tag == TAG_QUIT) {
      client->conn->send(Message(TAG_OK, "bye"));
      delete client;
      return nullptr;
    }
    if (join.tag != TAG_JOIN) {
        client->conn->send(Message(TAG_ERR, "Must join a room first"));
        delete client;
        return nullptr;
    }
    // newline from room name
    std::string roomname = join.data;
    if (!roomname.empty() && roomname.back() == '\n') {
        roomname.pop_back();
    }
    Room* room = client->server->find_or_create_room(roomname);
    room->add_member(client->user);
    client->room = room;
    client->conn->send(Message(TAG_OK, "Successfully joined " + roomname));

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
// Server member function implementation
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

// Accept new connection and spawn worker
void Server::handle_client_requests() {
    while (true) {
        int fd = Accept(m_ssock, nullptr, nullptr);
        if (fd < 0) {
            std::cerr << "Accept failed\n";
            return;
        }
        Connection* conn = new Connection(fd);
        // Create new helper client thread data type to pass in info
        Client_thread* client = new Client_thread(this, conn);
        pthread_t tid;
        Pthread_create(&tid, nullptr, worker, client);
    }
}

Room* Server::find_or_create_room(const std::string& name) {
    Guard lock(m_lock);
    auto ex = m_rooms.find(name); //check if existed
    if (ex != m_rooms.end()) {
        //if the pointer to the existed room is not the end
        return ex->second;
    }
    Room* r = new Room(name);
    m_rooms[name] = r;
    return r;
}
