#include "tictactoe.hpp"
#include <cstdint>
#include <ncurses.h>
#include <string>

MainUI::MainUI(uint8_t _id) //id will probably always be 0?
  :Iwindow(9,30,winNames::mainUI, _id) //just an estimate for now
  {
    inputStr.fill(' ');
  }

void MainUI::drawInput(){// reqY is based on this or other sentences i guess
  addstr("IP: ");
  attron(COLOR_PAIR(2));
  addnstr(&inputStr[0],inputStr.max_size()-1); //wierd, can be undef behaviour. rewrite?
  attroff(COLOR_PAIR(2));
}

int MainUI::drawScreen(){
  switch (state) {
    case connStates::disconnected:{
      mvaddstr(drawLoc.y,drawLoc.x,"Disconnected");
      move(drawLoc.y+1,drawLoc.x);
      drawInput();
      break;
    }
    case connStates::vsAI: {
      mvaddstr(drawLoc.y,drawLoc.x,"You: ");
      addstr(std::to_string(wins).c_str());
      mvaddstr(drawLoc.y+1,drawLoc.x,"AI: ");
      addstr(std::to_string(losses).c_str());
      mvaddstr(drawLoc.y+2,drawLoc.x,"Disconnected");
      move(drawLoc.y+3,drawLoc.x);
      drawInput();

      break;
    }
    case connStates::waitingConn:{

      break;
    }
    case connStates::connected:{
      move(drawLoc.y,drawLoc.x);
      addstr("You: ");
      addstr(std::to_string(wins).c_str());
      move(drawLoc.y + 1,drawLoc.x);
      addstr("Other: "); //change to hostname in the future if possible
      addstr(std::to_string(losses).c_str());
      break;
    }
  //notifications

  //open new windows
  
  }
  return 0;
}

void MainUI::aiPlay(){

}
