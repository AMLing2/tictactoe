#include "tictactoe.hpp"
#include <ncurses.h>

Tttgame::Tttgame(int sqrSize_)
  :Iwindow(sqrSize_ * 3 + 2,sqrSize_ * 3 + 2) //sqrSize *3 + 2
  ,sqrSize{sqrSize_}
  { lineChar = has_colors()? ' ' : ' '; } //will need to change later

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
  //TODO: draw the player points here
  return 0;
}
