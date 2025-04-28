#include <pthread.h>
#include <iostream>
#include <sstream>
#include <memory>
#include <set>
#include <vector>
#include <cctype>
#include <cassert>
#include "message.h"
#include "connection.h"
#include "user.h"
#include "room.h"
#include "guard.h"
#include "server.h"

////////////////////////////////////////////////////////////////////////
// Server implementation data types
////////////////////////////////////////////////////////////////////////

// TODO: add any additional data types that might be helpful
//       for implementing the Server member functions

////////////////////////////////////////////////////////////////////////
// Client thread functions
////////////////////////////////////////////////////////////////////////

namespace {
struct Client_thread{
  Server* server;
  Connection* conn;
};
void chat_with_sender(Connection* conn, Room* room, User* user){
  return;
}
void chat_with_receiver(Connection* conn, Room* room, User* user){
  return;
}
void *worker(void *arg) {
  pthread_detach(pthread_self());

  // TODO: use a static cast to convert arg from a void* to
  //       whatever pointer type describes the object(s) needed
  //       to communicate with a client (sender or receiver)
  Client_thread* client_conn = static_cast<Client_thread*>(arg);
  Connection* conn = client_conn->conn;
  Server* server = client_conn->server;

  // TODO: read login message (should be tagged either with
  //       TAG_SLOGIN or TAG_RLOGIN), send response
  Message receive_login;
  if (!conn->receive(receive_login)){
    //error handling
    delete conn;
    return nullptr;
  } 
  if (receive_login.tag == TAG_SLOGIN || receive_login.tag ==  TAG_RLOGIN){
    Message response_login;
    response_login.tag = TAG_OK;
    response_login.data = "logged in";
    if (!conn->send(response_login)){
      //error handling
      delete conn;
      return nullptr;
    } 
  } else{
    //error handling
    Message response_err_login;
    response_err_login.tag = TAG_ERR;
    response_err_login.data = "Invalid message.";
    conn->send(response_err_login);
    delete conn;
    return nullptr;
  }
  Message receive_join;
  if (!conn->receive(receive_join)){
   //error handling
   delete conn;
   return nullptr;
  }
  if(receive_join.tag == TAG_JOIN){
    Message response_join;
    response_join.tag = TAG_OK;
    response_join.data = "joined";
    if (!conn->send(response_join)){
      //error handling
      delete conn;
      return nullptr;
    }
  } else{
    //error handling
    Message response_err_join;
    response_err_join.tag = TAG_ERR;
    response_err_join.data = "Invalid message.";
    conn->send(response_err_join);
    delete conn;
    return nullptr;
  }

  // TODO: depending on whether the client logged in as a sender or
  //       receiver, communicate with the client (implementing
  //       separate helper functions for each of these possibilities
  //       is a good idea)
  User* user = new User(receive_login.data);
  Room* room = server->find_or_create_room(receive_join.data);
  room->add_member(user); 
  if (receive_login.tag == TAG_SLOGIN){
    chat_with_sender(conn, room, user);
  }else{
    chat_with_receiver(conn, room, user);
  }
  room->remove_member(user);
  delete user;
  delete client_conn;
  delete conn;
  return nullptr;
}

}

////////////////////////////////////////////////////////////////////////
// Server member function implementation
////////////////////////////////////////////////////////////////////////

Server::Server(int port)
  : m_port(port)
  , m_ssock(-1) {
  // initialize mutex
  pthread_mutex_init(&m_lock, nullptr);
}

Server::~Server() {
  // destroy mutex
  pthread_mutex_destroy(&m_lock);
}

bool Server::listen() {
  // use open_listenfd to create the server socket, return true
  // if successful, false if not
  std::string portstr = std::to_string(m_port);
  m_ssock = open_listenfd(portstr.c_str());
  if (m_ssock < 0){
    return false;
  }
  return true;
}

void Server::handle_client_requests() {
  // TODO: infinite loop calling accept or Accept, starting a new
  //       pthread for each connected client
  while(1){
    int client_fd = Accept(m_ssock, nullptr, nullptr);
    Connection* conn = new Connection(client_fd);
    pthread_t thread;
    Client_thread* client_conn = new Client_thread();
    client_conn->server = this;
    client_conn->conn = conn;
    Pthread_create(&thread, nullptr, worker, client_conn);
  }
}

Room *Server::find_or_create_room(const std::string &room_name) {
  // return a pointer to the unique Room object representing
  // the named chat room, creating a new one if necessary
  Guard lock(m_lock);
  auto ex = m_rooms.find(room_name); //check if existed
  if (ex != m_rooms.end()) { //if the pointer to the existed room is not the end
    return ex->second;
  }
  Room* r = new Room(room_name);
  this->m_rooms[room_name] = r;
  return r;
}
