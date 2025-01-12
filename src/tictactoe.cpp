#include <algorithm>
#include <csignal>
#include <cstdint>
#include <curses.h>
#include "tictactoe.hpp"
#include <memory>
#include <mutex>
#include <ncurses.h>
#include <vector>
#include <iostream>

//err handling
void INTHandler(int sig){
  signal(sig, SIG_IGN);//ignore signal
  mainLoopRun = 0;
}

//Iwindow functions, move in future? ended up with more than expected
bool Iwindow::checkIdNameMatch(winNames _name, uint8_t _id){
  //we only care about both matching, add +1 to newID when true
  return ((_name == name) & (_id == id))? true : false;
}

bool Iwindow::checkIfFit(const int desX, const int max_X){
  //this kind of seems a little pointless?, desY not used since no max
  if (desX == 0){ // & desY == 0
    return true; //draw top left window it always, all with desX = 0 should be drawn?
  }
  else if ((desX + drawLoc.reqX + 1) > max_X) {
    return false;
  }
  return true;
}

int Iwindow::moveWin(const int newY, const int newX){
  if ((newY < 0) | (newX < 0)){ return 1;} // will probably never hit this
  drawLoc.y = newY;
  drawLoc.x = newX;
  return 0;
}

bool Iwindow::cursorOnWindow(int y, int x){
  if ( ((x >= drawLoc.x) & (x <= (drawLoc.x + drawLoc.reqX)))&
        ((y >= drawLoc.y) & (y <= (drawLoc.y + drawLoc.reqY)))){
    handleCursorPress(y, x);
    return true;
  }
  else {
    return false;
  }
}
/*
call at the end of drawScreens()
 */
/* //TODO:
void drawBorder(winVec_t& vWindows){
  for (std::unique_ptr<Iwindow>& scrWin : vWindows){
    if(!
       
    )
  }
}
*/
/*
needs to be called before first drawScreens()
also call after any terminal resize or new windows added
*/
void moveScreens(winVec_t& vWindows,const int max_X){
  //need fix after +(1,1) was added
  int prevX = 1;
  int prevY = 1;
  int lowestY = 1; //rename to highest? lowest as in bottom of screen
  for( std::unique_ptr<Iwindow>& scrWin : vWindows){
    if (!scrWin->checkIfFit(prevX, max_X)){ // move window under
      scrWin->moveWin(lowestY,1); //will draw first in 0,0
      prevX = 1; //reset x and y
      prevY = lowestY;
      lowestY = scrWin->getHeight(); 
    }
    else {
      scrWin->moveWin(prevY, prevX);
      prevX = scrWin->getWidth();
      if (lowestY < scrWin->getHeight()){
        lowestY = scrWin->getHeight();
      }
    }
  }
}

void drawScreens(winVec_t& vWindows)
{
  int prevX;
  int prevY;
  const std::lock_guard<std::mutex> lock(drawMtx);
  getyx(stdscr,prevY,prevX);
  //this method is really inneficient due to the linked-list effect of objects
  //however functionally it works pretty well
  for( std::unique_ptr<Iwindow>& scrWin : vWindows){
    scrWin->drawScreen();
  }
  move(prevY,prevX);
  refresh();
}

template <typename T>
int addNewWin(winVec_t& vWindows, conn_t& conn){ //,winNames _name
  /*
  int newID = 0;
  for (std::unique_ptr<Iwindow>& scrWin : vWindows){
    if (scrWin->checkIdNameMatch(_name, newID)){
      newID++;
    }
  }
  */
  int newID = vWindows.size(); //ID is no longer window specific
  vWindows.push_back(std::make_unique<T>(newID,conn));
  /*
  //could be done better with a template type but constructors
  //might be wierd then, might make all constructor have ID as the only arg
  //in the future then i will change to that method instead.
  //current constructor inputs other than the id are magic numbers anyways...
  switch (_name) {
    case winNames::tictactoe:{
      vWindows.push_back(std::make_unique<Tttgame>(newID));
      return 0;
    }
    case winNames::mainUI:{
      vWindows.push_back(std::make_unique<MainUI>(newID));
      return 0;
    }
  }
  */
  return 1;
}

int passKbchar(const char c, winVec_t& vWindows){

  return 0;
}

void initGameScr(int& max_y, int& max_x)
{
  initscr();
  noecho();
  keypad(stdscr,true); //needed for mouse to work for some reason
  mmask_t mprev = mousemask(BUTTON1_RELEASED, NULL);//enable mouse L & R
  mprev = mousemask(BUTTON3_RELEASED, &mprev); //3 = right btn
  mprev = mousemask(BUTTON2_RELEASED, &mprev); //3 = right btn
  scrollok(stdscr,true); //allow scrolling if terminal is too small
  
  if (true){ //fix has_color() returning false
    start_color();
    //            foreground   background
    init_pair(0, COLOR_WHITE, COLOR_BLACK); //background or generic
    init_pair(1, COLOR_WHITE, COLOR_RED); //gamelines or borders
    init_pair(2, COLOR_BLACK, COLOR_CYAN); //input fields
  }
  move(0,0);
  getmaxyx(stdscr, max_y, max_x);

}

int inputLoop(winVec_t& vWindows, conn_t& pconn){
  int ch;
  char chChar;
  MEVENT mevent;
  iMsg::baseMsg<uint8_t> tempMsg; //temp
  tempMsg.action = msgActions::newData;
  tempMsg.winID = 0;
  std::string msgStr = "hellooo234";
  std::copy(msgStr.begin(),msgStr.end(),std::back_inserter(tempMsg.data));

  while(mainLoopRun){
    ch = getch();
    if (((ch >= ' ' ) & (ch <= '}')) //acceptable standard keyboard chars
    & (ch != '\\') & (ch != '/')){ //could be abused?
      chChar = static_cast<char>(ch & 0xFF);
      passKbchar(chChar, vWindows);
      if (pconn->sendMsgStruct<uint8_t>(false,tempMsg) < 0){
        return 1;
      }
      continue;
    }

    switch (ch) {
      case KEY_MOUSE:{
        if (getmouse(&mevent) == OK){
          move(mevent.y,mevent.x);
          if ((mevent.bstate & BUTTON1_RELEASED) | true){ // always for now
            //move(mevent.y,mevent.x);
            for( std::unique_ptr<Iwindow>& scrWin : vWindows){
              scrWin->cursorOnWindow(mevent.y,mevent.x);
            }
            refresh();
          }
        }
        break;
      }
    }
  }
  return 0;
}

int main () { //int argc, char *argv[]
  signal(SIGINT, INTHandler);
  int max_y, max_x;
  winVec_t vOpenWindows;
  conn_t pConnector = std::make_unique<Connector>(vOpenWindows);
  //int a;
  //std::cin>>a; //for debugging: wait for enter to start
  initGameScr(max_y, max_x);
  //temporary:
  addNewWin<Tttgame>(vOpenWindows, pConnector);
  addNewWin<MainUI>(vOpenWindows, pConnector);
  addNewWin<Tttgame>(vOpenWindows, pConnector);
  addNewWin<Tttgame>(vOpenWindows, pConnector);
  /*
  vOpenWindows.push_back(std::make_unique<Tttgame>(0)); //todo, make number based on something
  vOpenWindows.push_back(std::make_unique<MainUI>(0));
  */

  moveScreens(vOpenWindows,max_x);
  drawScreens(vOpenWindows);

  int r = inputLoop(vOpenWindows,pConnector);
  //finish curses, move to function later
  pConnector->endThread(false);
  endwin();
  std::cout << "rval: " << r <<" errno: "<< errno <<" EOMdet: "<<tmpEOMdet<< std::endl;
  return 0;
}
