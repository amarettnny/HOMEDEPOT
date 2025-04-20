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

  // TODO: connect to server
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()){
    std::cerr << "Could not connect to the server" << std::endl;
    return 1;
  }
  // TODO: send rlogin and join messages (expect a response from
  //       the server for each one)
  Message msg_login = Message(TAG_RLOGIN, username); 
  conn.send(msg_login);
  Message receive_login;
  conn.receive(receive_login);
  if (receive_login.tag == TAG_ERR){
    std::cerr << receive_login.data << std::endl;
    return 1;
  }
  if (receive_login.tag != TAG_OK){
    //std::cerr << "unexpected message received" << std::endl;
    return 1;
  }
  //receive_login.tag == TAG_OK
  Message msg_join = Message(TAG_JOIN, room_name);
  conn.send(msg_join);
  Message receive_join;
  conn.receive(receive_join);
  if(receive_join.tag == TAG_ERR){
    std::cerr << receive_join.data << std::endl;
    return 1;
  }
  if(receive_join.tag != TAG_OK){
    //std::cerr << "unexpected message received" << std::endl;
    return 1;    
  }
  
  // TODO: loop waiting for messages from server
  //       (which should be tagged with TAG_DELIVERY)
  while(1){
    Message msg_deliver;
    conn.receive(msg_deliver);
    if (msg_deliver.tag == TAG_ERR){
      std::cerr << msg_deliver.data << std::endl;
    }else if (msg_deliver.tag != TAG_DELIVERY){
      //std::cerr << "unexpected message received" << std::endl;
    } else{
      std::string delivery = msg_deliver.data;
      size_t split_index = delivery.find(":");
      std::string output = delivery.substr(split_index + 1);
      std::cout << output << std::endl;
    }
  }

  return 0;
}
