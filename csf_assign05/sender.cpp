#include <iostream>
#include <string>
#include <sstream>
#include <stdexcept>
#include "csapp.h"
#include "message.h"
#include "connection.h"
#include "client_util.h"

bool setMessage(const std::string &line, Message &outmsg) {
    std::istringstream iss(line);
    std::string cmd;
    iss >> cmd;

    // Handle different cases of commands
    if (cmd == "/join") {
        std::string room;
        if (!(iss >> room)) {
            std::cerr << "Usage: /join [room]" << std::endl;
            return false;
        }
        outmsg = Message(TAG_JOIN, room);
    }
    else if (cmd == "/leave") {
        outmsg = Message(TAG_LEAVE, "bye");
    }
    else if (cmd == "/quit") {
        outmsg = Message(TAG_QUIT,  "bye");
    }
    else if (cmd.size() > 0 && cmd[0] == '/') { // unknown command
        std::cerr << "Unknown command: " << cmd << std::endl;
        return false;
    }
    else {
        // plain text → sendall
        outmsg = Message(TAG_SENDALL, line);
    }

    return true;
}

int main(int argc, char **argv) {
  if (argc != 4) {
    std::cerr << "Usage: ./sender [server_address] [port] [username]\n";
    return 1;
  }

  std::string server_hostname;
  int server_port;
  std::string username;

  server_hostname = argv[1];
  server_port = std::stoi(argv[2]);
  username = argv[3];

  Connection conn;

  // connect to server
  conn.connect(server_hostname, server_port);
  if (!conn.is_open()){
    std::cerr << "Couldn't connect to the server" << std::endl;
    return 1;
  }

  // send slogin message
  Message msg_login = Message(TAG_SLOGIN, username); 
  if (!conn.send(msg_login)){
    std::cerr << "Error sending slogin" << std::endl;
    return 1;
  }

  Message receive_login;
  if (!conn.receive(receive_login)) {
    std::cerr << "Couldn't receive login message" << std::endl;
    return false;
  }
  if (receive_login.tag == TAG_ERR) {
    std::cerr << receive_login.data << std::endl;
    return 1;
  }
  if (receive_login.tag != TAG_OK) {
    std::cerr << "unexpected message received" << std::endl;
    return 1;
  }

  // loop reading commands from user, sending messages to
  // server as appropriate
  std::string line;
  while (1){
    std::getline(std::cin, line); // get the user input
    if (line.empty()) {
      continue;
    }

    Message outmsg;
    if (!setMessage(line, outmsg)) {
        // invalid command  -> skip
        continue;
    }

    // special handling of /quit
    if (outmsg.tag == TAG_QUIT) {
      conn.send(outmsg);
      Message quit_rsp;
      if (conn.receive(quit_rsp) && quit_rsp.tag == TAG_ERR) {
        std::cerr << quit_rsp.data << std::endl;
      }
      return 0;
    }

    // otherwise send and wait for ok/err
    conn.send(outmsg);
    Message rsp;
    if (conn.receive(rsp) && rsp.tag == TAG_ERR) {
      std::cerr << rsp.data << std::endl;
    }
  }

  return 0;
}