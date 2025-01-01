#include "tictactoe.hpp"
#include <asm-generic/socket.h>
#include <memory>
#include <netinet/in.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>

int Connector::startClientorInstance(bool isClient, bool replacePtr){
  if (replacePtr & (serverClient != nullptr)){
    //swap state between client <-> instance, isClient is ignored unless nullptr
    if (serverClient->isInstance){
      serverClient.reset(
        std::make_unique<Connector::ClientSocket>().release());
      //bascially the same as .reset(new Connector::ClientSocket) but slightly safer
    }
    else{
      serverClient.reset(
        std::make_unique<Connector::InstanceSocket>().release());
    }
    return 0;
  }
  else if (serverClient != nullptr){
    return 1;
  }
  //create unitialized
  //serverClient = isClient? std::make_unique<Connector::ClientSocket>()
  //                        :std::make_unique<Connector::InstanceSocket>();
  //short if didnt work :'(
  if(isClient){
    serverClient = std::make_unique<Connector::ClientSocket>();
  }
  else{
    serverClient = std::make_unique<Connector::InstanceSocket>();
  }
  return 0;
}

int Connector::loopbackData(){

  return 0;
}

int ABserverClient::startSocket(){
  /*
  struct addrinfo hints;
  std::memset(&hints,0,sizeof(hints));
  hints.ai_family = AF_INET;
  hints.ai_socktype = SOCK_STREAM;
  hints.ai_protocol = IPPROTO_TCP;
  */

  serverAddress.sin_family = AF_INET;
  serverAddress.sin_addr.s_addr = INADDR_ANY;
  serverAddress.sin_port = htons(instPort);
  selfSocket = socket(AF_INET,
                      SOCK_STREAM | SOCK_CLOEXEC,
                      IPPROTO_TCP);
  //allow reuse of port
  int reusenum = 1;
  setsockopt(selfSocket, SOL_SOCKET, SO_REUSEPORT, &reusenum, 1);
  if (selfSocket < 0){
    return 1;
  }
  if (isInstance){ //bind, might have to change this later, might be for local only
    if (bind(selfSocket, reinterpret_cast<struct sockaddr*>(&serverAddress),
         sizeof(serverAddress)) != 0){
      return 1;
    }
    if (listen(selfSocket, maxConnections) != 0){
      return 1;
    }
  }
  return 0;
}

void Connector::mainLoop(){ //ran in a separate thread
  //restart thread on serverClient change
  while(mainLoopRun){

    if (serverClient != nullptr){

    }
  }
}

//constructors for scoped classes
Connector::InstanceSocket::InstanceSocket()
  :ABserverClient(true){

}

Connector::ClientSocket::ClientSocket()
  :ABserverClient(false){

}

//functions for scoped classes

int Connector::InstanceSocket::mainFunc(){
  return 0;
}

int Connector::ClientSocket::mainFunc(){
  //connect
  return 0;
}
