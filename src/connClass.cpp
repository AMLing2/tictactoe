#include "tictactoe.hpp"
#include <array>
#include <asm-generic/socket.h>
#include <cerrno>
#include <cmath>
#include <memory>
#include <ncurses.h>
#include <netinet/in.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netdb.h>
#include <cstring>
#include <thread>
#include <poll.h>
#include <unistd.h>
#include <mutex>
#include <iostream>
/* remove an FD from list and fix remaining list */
void fdArrRem(struct pollfd* conFDs,const nfds_t i, nfds_t& conFD_cur){
  if(conFD_cur != 0){//its an unsigned int type so we do not want to -- it at 0
    if (conFD_cur == (i + 1)){//if end of used array
      conFDs[i].fd = -1; //initialize to empty
      conFDs[i].events = 0;
      conFDs[i].revents = 0;
    }
    else{
      for (nfds_t n = i; n < (conFD_cur - 1); n++){
        conFDs[n] = conFDs[n+1];
      }
      conFDs[conFD_cur - 1].fd = -1; //initialize end to empty
      conFDs[conFD_cur - 1].events = 0;
      conFDs[conFD_cur - 1].revents = 0;
    }
    conFD_cur--;
  }
}

bool checkBitSet(int toCheck, int bit){//for comparing C bit macros
  return true; //TODO: make
}

int Connector::startClientorInstance(bool isClient, bool replacePtr){
  if (replacePtr & (serverClient != nullptr)){
    //swap state between client <-> instance, isClient is ignored unless nullptr
    if (serverClient->isInstance){
      serverClient.reset(
        std::make_unique<Connector::ClientSocket>(pipeFD[0]).release());
      //bascially the same as .reset(new Connector::ClientSocket) but slightly safer
    }
    else{//pipeFD[0] = read end, [1] = write end
      serverClient.reset(
        std::make_unique<Connector::InstanceSocket>(MAXCONNECTIONS_,
                                                    pipeFD[0]).release());
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
  if (isClient){
    serverClient = std::make_unique<Connector::ClientSocket>(pipeFD[0]);
  }
  else{
    serverClient = std::make_unique<Connector::InstanceSocket>(MAXCONNECTIONS_,
                                                               pipeFD[0]);
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
                      SOCK_STREAM | SOCK_CLOEXEC | SOCK_NONBLOCK,
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
  else{
  }
  return 0;
}
void ABserverClient::startThread(){
  threadLoopRun = true; //intiialized twice here?
  tThread = std::thread([this](){
    mainLoop();
  });
}

int Connector::endThread(bool restartT){
  serverClient->threadLoopRun = false;
  const size_t bufSize = 2;
  char buf[bufSize] = {'\u0019','a'}; //EOM,\0
  writeToPipe(buf, bufSize);
  if (serverClient->tThread.joinable()){
    serverClient->tThread.join();
  }
  if (restartT){
    serverClient->startThread(); //NOTE:not sure if this will be used ever
  }
  return 0;
}

int Connector::writeToPipe(char* buff,size_t bufflen){
  return write(pipeFD[1], buff, bufflen);
}

void ABserverClient::mainLoop(){
  //uses poll() to recieve any data from clients or local, where local is
  //through a self-pipe
  //instance specific: echo all recv data to all connected clients
  int pollRN = 0;
  int fdHandles = 0;
  int pollTimeout;
  if (isInstance){
    pollTimeout = -1;
  } 
  else{
    pollTimeout = TIMEOUT_;
  }

  int a = 0; //temp
  while(threadLoopRun){
    pollRN = poll(conFDs, conFDsn_cur, pollTimeout);//-1 for infinite block
    a++;
    move(5,5);
    addstr(std::to_string(a).c_str());
    refresh();

    if (pollRN == -1){//error, not sure what to do here for now
      move(7,2);
      char str[19] = {"ERROR!!! !!!!!!!!!"};
      addstr(str);
      refresh();
      break;
    }
    else if (pollRN == 0){ //timeout, check accpetFunc()
      //NOTE: there is a way to use poll() to check for accept rather than
      //polling it manually, but i cant find anywhere in the manpages what
      //FD to poll to check this, it only mentions that its possible
      acceptFunc();
      continue;
    }
    else{//data to read
      //loop through conFDs.revents to find which FD notified
      fdHandles = 0; //break if all the FDs were handled, no point checking rest
      for (nfds_t i = 0; (i < conFDsn_cur) & 
                      (fdHandles < pollRN); i++){
        if (conFDs[i].revents != 0){
          fdHandles++;
          switch(conFDs[i].revents){
            case POLLHUP: {//server killed connection, remove FD
              close(conFDs[i].fd);
              fdArrRem(conFDs,i,conFDsn_cur);
              break;
            }
            case POLLIN: { //data to be read
              if (i == 0){ //pipe
                bufRecvn = read(conFDs[i].fd, recvBuffer.data(), recvBuffer.size());
                //if ((recvBuffer[0] == '\u0019') & (bufRecvn == 2)){
                if (recvBuffer[0] == '\u0019'){ //FIX: need to make this better
                  tmpEOMdet = 1;
                  threadLoopRun = false;//should this be done only through endThread()?
                  break;
                }
                else { //temp
                  const std::lock_guard<std::mutex> lock(drawMtx);
                  move(1,1);
                  for (size_t i = 0; i < bufRecvn; i++){
                    addch(recvBuffer[i]); //TODO:temp
                  }
                  refresh();
                }
              }
              else {//socket

              }
            }
          }
        }
        /*
        else { //revents is zero, ignore

        }
        */
      }
    }
  }
}

int Connector::InstanceSocket::acceptFunc(){
  socklen_t conAddrLen = 0;
  struct sockaddr conAddr;
  int acceptFD = 0;
  std::memset(&conAddr,0,sizeof(conAddr));

  acceptFD = accept(selfSocket,
         &conAddr, //TODO: not using this for anything currently, maybe log?
         &conAddrLen);
  //add to conFDs list
  if (acceptFD < 0){
    if ((errno == EAGAIN) | (errno == EWOULDBLOCK)){
      return 1; //no accept pending
    }
    else{
      //actual error, ignore for now
    }
  }
  else { // add to poll list
    conFDs[conFDsn_cur].fd = acceptFD;
    conFDs[conFDsn_cur].events = POLLIN | POLLHUP;
    conFDs[conFDsn_cur].revents = 0; //initialize return to zero
    conFDsn_cur++;
  }
  return 0;
}

int Connector::ClientSocket::acceptFunc(){
  return 0; //do nothing, wont be called as poll() wont timeout as client
}

Connector::Connector(winVec_t& _vWindows)
  :vWindows{_vWindows} {
  std::cout<< "pipe R:" << pipe(pipeFD)<<std::endl; // should check err, but very rare
  startClientorInstance(true, false); //always start as client
}

//constructors for scoped classes
ABserverClient::ABserverClient(bool isInstance_,int maxConns_, int pipeReadFD)
  :isInstance(isInstance_)
  ,maxConnections(maxConns_) //socket connections
  ,conFDsn_max(maxConns_ + 1) //+1 for self pipe
{
  startSocket();//NOTE:should this be done automatically as client?
  conFDs = new struct pollfd[maxConnections + 1];//+1 for self pipe
  conFDs[0].fd = pipeReadFD; //set first FD to pipe
  conFDs[0].events = POLLIN; //check for data to read
  conFDs[0].revents = 0; //initialize return to zero, see asm-generic/poll.h
  conFDsn_cur = 1;
}

Connector::InstanceSocket::InstanceSocket(int maxConns_, int pipeReadFD)
  :ABserverClient(true, maxConns_, pipeReadFD)
{
  startThread(); //move to ABserverClient constructor?
}

Connector::ClientSocket::ClientSocket(int pipeReadFD)
  :ABserverClient(false, 1, pipeReadFD)
{
  startThread();
}
//destructors
Connector::InstanceSocket::~InstanceSocket(){
  close(selfSocket);
  delete conFDs;
}

Connector::ClientSocket::~ClientSocket(){
  close(selfSocket);
  delete conFDs;
}

Connector::~Connector(){
  close(pipeFD[0]);
  close(pipeFD[1]);
  endThread(false);
}
