#include <iostream>
#include <string>
#include <vector>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

int main(int argc, char **argv) {
  if (argc != 5) {
    std::cerr << "Usage: ./receiver [server_address] [port] [username] [room]\n";
    return 1;
  }

  std::string server_hostname = argv[1];
  int server_port = std::stoi(argv[2]);
  std::string username = argv[3];
  std::string room_name = argv[4];

  Connection conn;

  // connect to the server
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()){
    std::cerr << "Could not connect to the server" << std::endl;
    return 1;
  }
  // send rlogin and join messages
  Message msg_login = Message(TAG_RLOGIN, username); 
  if(!conn.send(msg_login)){
    std::cerr << "cannot send message" << std::endl;
    return 1;
  }
  Message receive_login;
  if(!conn.receive(receive_login)){
    std::cerr << "cannot receive message" << std::endl;
    return 1;
  }
  if (receive_login.tag == TAG_ERR){
    std::cerr << receive_login.data << std::endl;
    return 1;
  }
  if (receive_login.tag != TAG_OK){
    std::cerr << "unexpected message received" << std::endl;
    return 1;
  }
  //receive_login.tag == TAG_OK
  Message msg_join = Message(TAG_JOIN, room_name);
  if(!conn.send(msg_join)){
    std::cerr << "cannot send message" << std::endl;
    return 1;
  }
  Message receive_join;
  if(!conn.receive(receive_join)){
    std::cerr << "cannot receive message" << std::endl;
  }
  if(receive_join.tag == TAG_ERR){
    std::cerr << receive_join.data << std::endl;
    return 1;
  }
  if(receive_join.tag != TAG_OK){
    std::cerr << "unexpected message received" << std::endl;
    return 1;    
  }
  //receive_join.tag == TAG_OK, can receive message delivered now
  //waiting for messages from server
  while(1){
    // return if server is closed
    if (!conn.is_open()){
      std::cerr << "could not connect to the server" << std::endl;
      return 1;
    }
    Message msg_deliver;
    conn.receive(msg_deliver);
    if (msg_deliver.tag == TAG_ERR){
      std::cerr << msg_deliver.data << std::endl;
    }else if (msg_deliver.tag != TAG_DELIVERY){
      continue;
    } else{
      //split the delivered message
      std::string delivery = msg_deliver.data;
      size_t split_index = delivery.find(":");
      std::string output = delivery.substr(split_index + 1);
      std::string room_check = delivery.substr(0, split_index);
      //check if the message is sent to the current room
      if (room_check == room_name){
	size_t split_index2 = output.find(":");
	std::string sender_name = output.substr(0, split_index2);
	std::string message_send = output.substr(split_index2 + 1);
	//print message in desired format
        std::cout << sender_name + ": " + message_send << std::endl;
      }else{
        continue;
      }
    }
  }

  return 0;
}
