#include <csignal>
#include <cstdint>
#include <curses.h>
#include "tictactoe.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

static volatile sig_atomic_t mainLoopRun{1};
//err handling
void INTHandler(int sig){
  signal(sig, SIG_IGN);//ignore signal
  mainLoopRun = 0;
}

//functions
bool Iwindow::checkIdNameMatch(winNames _name, uint8_t _id){
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
  for( std::unique_ptr<Iwindow>& scrWin : vWindows){
    scrWin->drawScreen();
  }
  refresh();
}

void initGameScr(int& max_y, int& max_x)
{
  initscr();
  scrollok(stdscr,true); //allow scrolling if terminal is too small
  if (true){ //fix has_color() returning false
    start_color();
    //            foreground   background
    init_pair(0, COLOR_WHITE, COLOR_BLACK); //background or generic
    init_pair(1, COLOR_WHITE, COLOR_RED); //gamelines or borders
    init_pair(2, COLOR_BLACK, COLOR_CYAN); //input fields
  }
  getmaxyx(stdscr, max_y, max_x);

}

int inputLoop(){
  while(mainLoopRun){

  }
  return 0;
}

int main () { //int argc, char *argv[]
  signal(SIGINT, INTHandler);
  int max_y, max_x;
  initGameScr(max_y, max_x);

  winVec_t vOpenWindows;
  vOpenWindows.push_back(std::make_unique<Tttgame>(4,0)); //todo, make number based on something
  vOpenWindows.push_back(std::make_unique<MainUI>(0));

  moveScreens(vOpenWindows,max_x);
  drawScreens(vOpenWindows);

  inputLoop();
  //finish curses, move to function later
  endwin();
  return 0;
}
