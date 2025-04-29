#include "guard.h"
#include "message.h"
#include "message_queue.h"
#include "user.h"
#include "room.h"

Room::Room(const std::string &room_name)
  : room_name(room_name) {
  // initialize the mutex
  pthread_mutex_init(&lock, nullptr);
}

Room::~Room() {
  // destroy the mutex
  pthread_mutex_destroy(&lock);
}

void Room::add_member(User *user) {
  // add User to the room
  {
    Guard lock(this->lock);
    members.insert(user);
  }
}

void Room::remove_member(User *user) {
  // remove User from the room
  {
    Guard lock(this->lock);
    members.erase(user);
  }
} 

void Room::broadcast_message(const std::string &sender_username, const std::string &message_text) {
  Guard lock(this->lock);
  // send a message to every (receiver) User in the room
  // format the broadcast message
  std::string msg_str = this->room_name + ":" + sender_username + ": " + message_text;

  // for each user enqueue a freshly-allocated Message
  for (User *u : members) {
    Message *msg = new Message(TAG_DELIVERY, msg_str);
    u->mqueue.enqueue(msg);
  }
}
