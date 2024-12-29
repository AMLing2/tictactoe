#include <curses.h>
#include "tictactoe.hpp"
#include <memory>
#include <vector>
#include <thread>
#include <chrono>

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
needs to be called before drawScreens()
call after any terminal resize or new windows added
*/
void moveScreens(std::vector<std::unique_ptr<Iwindow>>& vWindows,const int max_X){
  int prevX = 0;
  int prevY = 0;
  int lowestY = 0; //rename to highest? lowest as in bottom of screen
  for( std::unique_ptr<Iwindow>& scrWin : vWindows){
    if (!scrWin->checkIfFit(prevX, max_X)){ // move window under
      scrWin->moveWin(lowestY,0); //will draw first in 0,0
      prevX = 0; //reset x and y
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

void drawScreens(std::vector<std::unique_ptr<Iwindow>>& vWindows)
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
  }
  getmaxyx(stdscr, max_y, max_x);

}

int main () { //int argc, char *argv[]
  int max_y, max_x;
  initGameScr(max_y, max_x);

  std::vector<std::unique_ptr<Iwindow>> vOpenWindows;//not sure if this will work
  vOpenWindows.push_back(std::make_unique<Tttgame>(5)); //todo, make number based on something
  moveScreens(vOpenWindows,max_x);
  drawScreens(vOpenWindows);

  std::this_thread::sleep_for(std::chrono::seconds(4));

  //finish curses
  endwin();
  return 0;
}
