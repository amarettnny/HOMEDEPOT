#include <sstream>
#include <cctype>
#include <cassert>
#include "csapp.h"
#include "message.h"
#include "connection.h"

Connection::Connection()
  : m_fd(-1)
  , m_last_result(SUCCESS) {
}

Connection::Connection(int fd)
  : m_fd(fd)
  , m_last_result(SUCCESS) {
  // call rio_readinitb to initialize the rio_t object
  rio_readinitb(&m_fdbuf, fd);
}

void Connection::connect(const std::string &hostname, int port) {
  //call open_clientfd to connect to the server
  std::string port_str = std::to_string(port);  
  int socket_descriptor = open_clientfd(hostname.c_str(), port_str.c_str());
  if (socket_descriptor == -1 || socket_descriptor == -2){
    m_last_result = EOF_OR_ERROR;
    m_fd = -1;
    return;
  }
  //call rio_readinitb to initialize the rio_t object
  m_fd = socket_descriptor;
  m_last_result = SUCCESS;
  rio_readinitb(&m_fdbuf, m_fd);
}

Connection::~Connection() {
  //close the socket if it is open
  if (is_open()){
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::is_open() const {
  //return true if the connection is open
  if (m_fd >= 0){
    return true;
  }
  return false;
}

void Connection::close() {
  //close the connection if it is open
  if (is_open()){
    Close(m_fd);
    m_fd = -1;
  }
}

bool Connection::send(const Message &msg) {
  // send a message return true if successful, false if not
  // set m_last_result appropriately
  std::string data = msg.data;
  std::string tag = msg.tag;
  // check encoded message is within the required length  
  if ((data.length() + 2 + tag.length()) > Message::MAX_LEN){
    m_last_result = INVALID_MSG;
    return false;
  }
  // check tag is valid
  if(tag != TAG_ERR && tag != TAG_OK && tag != TAG_SLOGIN && tag != TAG_RLOGIN && 
		  tag != TAG_JOIN && tag != TAG_LEAVE && tag != TAG_SENDALL && 
		  tag != TAG_SENDUSER && tag != TAG_QUIT && tag != TAG_DELIVERY
		  && tag != TAG_EMPTY){
    m_last_result = INVALID_MSG;
    return false;
  }
  // encoded the message and send
  std::string send_message = msg.tag + ":" + msg.data + "\n";
  ssize_t size_write = rio_writen(m_fd, send_message.c_str(), send_message.length());
  if (size_write != (ssize_t)send_message.length()){
    m_last_result = EOF_OR_ERROR;
    return false;
  }else{
    m_last_result = SUCCESS;
    return true;
  }
}

bool Connection::receive(Message &msg) {
  // receive a message, storing its tag and data in msg
  // return true if successful, false if not
  // set m_last_result is appropriatel
  char receive_message[Message::MAX_LEN + 1];
  //read the message
  ssize_t size_read = rio_readlineb(&m_fdbuf, receive_message, Message::MAX_LEN);
  if (size_read <= 0){
    m_last_result = EOF_OR_ERROR;
    return false;
  }
  // delete the encoded new line character in the message
  if (receive_message[size_read - 1] == '\n'){
    receive_message[size_read - 1] = '\0';
  }else{
    receive_message[size_read] = '\0';
  }
  std::string tag;
  std::string data;
  std::string all_message(receive_message);
  //split the encoded message and store the tag and data
  size_t split_index = all_message.find(':');
  if (split_index == std::string::npos){
    m_last_result = EOF_OR_ERROR;
    return false;
  } else{
    tag = all_message.substr(0, split_index);
    data = all_message.substr(split_index + 1);
  }
  msg.tag = tag;
  msg.data = data;
  m_last_result = SUCCESS;
  return true;
}
