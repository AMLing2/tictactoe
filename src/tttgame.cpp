#include "tictactoe.hpp"
#include <algorithm>
#include <cstdint>
#include <iterator>
#include <ncurses.h>
#include <cmath>
#include <random>
#include <iostream>
#include <set>

Tttgame::Tttgame(uint8_t _id, conn_t& _Conn) //FIX: really needs cleaning
  :Iwindow(_id,4 * 3 + 2,4 * 3 + 2,
           winNames::tictactoe,  _Conn) //sqrSize *3 + 2
  //:Iwindow(sqrSize * 3 + 2,sqrSize * 3 + 2,
  //         winNames::tictactoe, _id, _Conn) //sqrSize *3 + 2
  {
    lineChar = has_colors()? ' ' : ' '; //will need to change later
    boardSpots.fill(' ');
  } 

int Tttgame::drawScreen(){
  //draw lines
  attron(COLOR_PAIR(1));
  for (int i = sqrSize; i<= sqrSize * 2; i+=sqrSize){
    move(drawLoc.y, drawLoc.x + i);
    vline(lineChar, drawLoc.reqY - 1);
    move(drawLoc.y + i, drawLoc.x);
    hline(lineChar, drawLoc.reqX - 1);
  }
  attroff(COLOR_PAIR(1));
  //draw player points:
  uint8_t i = 0;
  //can probably eliminate a loop, but it will prob be the same amount of instructions as it needs to move and addch 9 times either way
  for (uint8_t y = 0; y < 3; y++){
    for (uint8_t x = 0; x < 3; x++){//it works but.....
      move(std::lround((sqrSize/2) + (sqrSize * y) + y) + drawLoc.y -1,
           std::lround((sqrSize/2) + (sqrSize * x) + x) + drawLoc.x -1);
      addch(boardSpots[i]);
      i++;
    }
  }
  return 0;
}

int Tttgame::addToSpot(const uint8_t spot, const char c){
  if (spot > 8) {
    return 1;
  }
  else if (boardSpots[spot] != ' '){
    return 1;
  }
  else{
    boardSpots[spot] = c;
    const std::lock_guard<std::mutex> lock(drawMtx);
    drawScreen();
    return 0;
  }
}

int Tttgame::handleRecv(std::array<uint8_t,BUFFSIZE_>& msgBuf, size_t msgBytes){
  //NOTE: move struct conversion to function of iMSG?
  if ((msgBytes < 3) | (msgBytes > BUFFSIZE_)){ //redundant check?
    return  1;
  }
  iMsg::baseMsg<uint8_t> msgRecv;
  msgRecv.action = static_cast<msgActions>(msgBuf[0]);
  msgRecv.winID = msgBuf[1];//is there a point in filling in this?
  std::copy(msgBuf.begin()+2,
            msgBuf.begin()+msgBytes,
            std::back_inserter(msgRecv.data)); //most likely just 1 byte for the pos
  addToSpot(msgRecv.data[0],'X'); //'X' temporary?
  return 0;
}

void Tttgame::aiPlay(){//mimics instance, very bad "ai" for now, just a placeholder
  std::vector<int> usableIndexes;
  for (long unsigned int i = 0; i < boardSpots.size() ;i++){
    if (boardSpots.at(i) == ' '){
      usableIndexes.push_back(i);
    }
  }
  if (usableIndexes.size() != 0){
    std::random_device rd;
    std::mt19937 mt(rd());
    std::uniform_int_distribution<int> dist(0,usableIndexes.size()-1);
    addToSpot(usableIndexes.at(dist(mt)), 'X');
  }
  if(chkWin()){

  }
  waitingPlayer = false;
}

bool Tttgame::chkWin(){ //really want to clean this up
  //get unique characters and remove whitespace
  //a bit inefficient to do every loop?
  std::set<char> uniqueChars;
  //check horizontal
  int wIndex = -1;
  for (size_t i = 0; i <= boardSpots.size() - 3; i+=3){
    uniqueChars.insert(boardSpots.begin() + i, boardSpots.begin() + i + 3);
    if ((uniqueChars.size() == 1) &
      (uniqueChars.find(' ') == uniqueChars.end())){
      wIndex = i/3;
      break;
    }
    uniqueChars.clear();
  }
  if (wIndex != -1){ //draw hori line at index
    const std::lock_guard<std::mutex> lock(drawMtx);
    move(std::lround((sqrSize/2) + (sqrSize * wIndex) + wIndex) + drawLoc.y -1,
         drawLoc.x + 1);
    attron(COLOR_PAIR(2)); //cyan bg
    hline(lineChar, drawLoc.reqX - 3); //3 is temp
    attroff(COLOR_PAIR(1));
    refresh();
    return true;
  }
  //check vertical
  for (int i = 0; i < 3; i++){
    uniqueChars.insert(boardSpots.at(i));  //can be better
    uniqueChars.insert(boardSpots.at(i+3));
    uniqueChars.insert(boardSpots.at(i+6));
    if ((uniqueChars.size() == 1) &
      (uniqueChars.find(' ') == uniqueChars.end())){
      wIndex = i;
      break;
    }
    uniqueChars.clear();
  }
  if (wIndex != -1){ //draw vert line at index
    const std::lock_guard<std::mutex> lock(drawMtx);
    move(drawLoc.y + 1,
         std::lround((sqrSize/2) + (sqrSize * wIndex) + wIndex) + drawLoc.x -1);
    attron(COLOR_PAIR(2)); //cyan bg
    vline(lineChar, drawLoc.reqY - 3); //3 is temp
    attroff(COLOR_PAIR(1));
    refresh();
    return true;
  }
  //check diagonal
  for (int i = 0; i < 3; i+=2){
    uniqueChars.insert(boardSpots.at(i));
    uniqueChars.insert(boardSpots.at(4)); //always middle
    uniqueChars.insert(boardSpots.at(8 - i));
    if ((uniqueChars.size() == 1) &
      (uniqueChars.find(' ') == uniqueChars.end())){
      wIndex = i;
      break;
    }
    uniqueChars.clear();
  }
  if (wIndex != -1){ //no pretty way to draw diagonal lines with ncurses :(
    int x, y;
    int adder;
    const std::lock_guard<std::mutex> lock(drawMtx);
    if (wIndex == 0){
      x = std::lround(sqrSize/2) + drawLoc.x -1; //top left
      y = std::lround(sqrSize/2) + drawLoc.y -1;
      adder = 1;
    }
    else{
      y = std::lround(sqrSize/2) + drawLoc.y -1; //top right
      x = drawLoc.x -1 + drawLoc.reqX - std::lround(sqrSize/2); 
      adder = -1;
    }
    attron(COLOR_PAIR(2)); //cyan bg
    for(int i = 0; y < (drawLoc.y + drawLoc.reqY - std::lround(sqrSize/2));
        i+= adder){
      mvaddch(y, x, lineChar);
      y++;
      x += adder;
    }
    attroff(COLOR_PAIR(1));
    refresh();
    return true;
  }

  return false;
}

void Tttgame::handleCursorPress(const int cursy,const int cursx){
  if (waitingPlayer){ //wait for player response after pressing
    return;
  }
  const int relX = cursx - drawLoc.x;
  const int relY = cursy - drawLoc.y;
  int xPos = 0;
  int yPos = 0;
  //check press based on drawn lines' position
  for (int i = sqrSize; i<= sqrSize * 2; i+=sqrSize){
    if (relX > i){
      xPos++;
    }
    else if (relX == i) {
      return;//ignore if on line
    }
    if (relY > i){
      yPos++;
    }
    else if (relY == i) {
      return;
    }
  }
  const int posIndex = xPos + (3 * yPos);
  if(addToSpot(posIndex, 'O') == 0){ //ignore click if area already pressed
    //send press to others
    iMsg::baseMsg<uint8_t> msg;
    msg.winID = id;
    msg.action = newData;
    msg.data.push_back(posIndex);
    //conn->sendMsgStruct(false, msg); //FIX: uncomment
    waitingPlayer = true;
  
    //play with ai, will change in future to allow while still connected
    if ((conn->getConnStatus() == connStatusT::instNoConn) |
        (conn->getConnStatus() == connStatusT::clientNoConn)){
      aiPlay();
    }
  }
}
