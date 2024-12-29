#ifndef TICTACTOE
#define TICTACTOE

#include <ncurses.h>
#include <sys/socket.h>

struct ScreenLoc {
  const int reqY,reqX;
  int x{0};
  int y{0};
  ScreenLoc(int ySize, int xSize) : reqY(ySize) , reqX(xSize) {}
};

enum connState {
  
};

class Iconnection{

};

class Iwindow{
public:
  virtual ~Iwindow() = default;
  virtual int drawScreen() = 0; //pure virtual
  //virtual int updateFromRecv(char* buffer) = 0;
  bool checkIfFit(const int desX, const int max_X);
  int getHeight() {return drawLoc.y + drawLoc.reqY + 1;};
  int getWidth() {return drawLoc.x + drawLoc.reqX + 1;};
  int moveWin(const int newY, const int newX);
protected:
  ScreenLoc drawLoc;
  Iwindow(int yReq,int xReq) :drawLoc(yReq, xReq) {};
};
/*
class connScreen : public Iwindow{ 
public:
  //using state method design pattern for drawing
  //draws both win/losses of player and connection screen
  int drawScreen();

private:
  connState state;
  int attemptConnect();
  ScreenLoc drawLoc;
};
*/

class Tttgame : public Iwindow {
public:
  Tttgame(int sqrSize_);
  int drawScreen() override;
  //int updateFromRecv(char* buffer) override { return 0; };//temp
  /*
   s: 0-8, where 0 is top left
  */
  //~Tttgame(); 
private:
  char lineChar;
  const int sqrSize;
  void addToSpot(int s, char c);
  
};

#endif  //TICTACTOE
