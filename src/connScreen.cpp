#include "tictactoe.hpp"
#include <cstdint>
#include <ncurses.h>
#include <string>
#include <vector>

struct fieldData {
  std::vector<std::string> userNames;
  
};

MainUI::MainUI(uint8_t _id, conn_t& _Conn) //id will probably always be 0?
  :Iwindow(_id,9,30,winNames::mainUI,  _Conn) //just an estimate for now
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
  switch (conn->getConnStatus()) {
    case connStatusT::clientNoConn:{
      mvaddstr(drawLoc.y,drawLoc.x,"Disconnected");
      move(drawLoc.y+1,drawLoc.x);
      drawInput();
      break;
    }
    case connStatusT::clientConn: {
      mvaddstr(drawLoc.y,drawLoc.x,"You: ");
      addstr(std::to_string(wins).c_str());
      mvaddstr(drawLoc.y+1,drawLoc.x,"AI: ");
      addstr(std::to_string(losses).c_str());
      mvaddstr(drawLoc.y+2,drawLoc.x,"Disconnected");
      move(drawLoc.y+3,drawLoc.x);
      drawInput();

      break;
    }
    case connStatusT::instNoConn:{

      break;
    }
    case connStatusT::instConn:{
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

int MainUI::handleRecv(std::array<uint8_t,BUFFSIZE_>& msgBuf, size_t msgBytes){

  return 0;
}
void MainUI::handleCursorPress(const int cursy,const int cursx){

}
