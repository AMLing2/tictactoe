#include "tictactoe.hpp"
#include <cstdint>
#include <ncurses.h>
#include <cmath>

Tttgame::Tttgame(int sqrSize_,uint8_t _id)
  :Iwindow(sqrSize_ * 3 + 2,sqrSize_ * 3 + 2, winNames::tictactoe, _id) //sqrSize *3 + 2
  ,sqrSize{sqrSize_}
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
    return 0;
  }
}

void Tttgame::aiPlay(){
  
}
