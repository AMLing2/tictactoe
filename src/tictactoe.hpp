#ifndef TICTACTOE
#define TICTACTOE

#include <array>
#include <atomic>
#include <csignal>
#include <cstdint>
#include <ncurses.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include <thread>
#include <vector>
#include <atomic>
#include <mutex>

#define MAXCONNECTIONS_ 5
#define TIMEOUT_ 5000

class Connector;
class Iwindow;

typedef std::unique_ptr<Connector> conn_t;
typedef std::vector<std::unique_ptr<Iwindow>> winVec_t;

static volatile sig_atomic_t mainLoopRun{1};
inline std::mutex drawMtx;

enum class winNames : uint8_t {
tictactoe = 1,
mainUI = 2,
};

struct ScreenLoc {
  const int reqY,reqX; //might remove const as this might change
  int x{1}; // +1 for borders
  int y{1};
  ScreenLoc(int ySize, int xSize) : reqY(ySize) , reqX(xSize) {}
};

class ABserverClient{
public:
  virtual ~ABserverClient() = default;
  virtual int mainFunc() = 0;
  const bool isInstance;
  std::atomic<bool> threadLoopRun{true};
  std::thread tThread;
  void startThread();
protected:
  const nfds_t conFDsn_max;
  nfds_t conFDsn_cur;
  struct pollfd* conFDs;
  virtual void mainLoop() = 0;
  const int instPort{57384};
  const int maxConnections;
  int selfSocket;
  sockaddr_in serverAddress;
  int startSocket();
  ABserverClient(bool isInstance_,int maxConns_, int pipeReadFD);
};

class Connector{
public:
  int loopbackData();
  Connector(winVec_t& _vWindows);
  ~Connector();
  int endThread(int selfWriteFD, bool restartT);
private:
  int pipeFD[2]; // [0] = read end FD, [1] = write end FD
  int startClientorInstance(bool isClient, bool replacePtr);
  winVec_t& vWindows;
  std::unique_ptr<ABserverClient> serverClient;
//FIX: this is starting to get clunky and annoying, might just make it all
  //into one class
  class ClientSocket : public ABserverClient {
  public:
    ClientSocket(int pipeReadFD);
    ~ClientSocket();
    int mainFunc() override;
  private:
    void mainLoop() override;
  };
  class InstanceSocket : public ABserverClient {
  public:
    InstanceSocket(int maxConns_, int pipeReadFD);
    ~InstanceSocket();
    int mainFunc() override;
  private:
    void mainLoop() override;
  };
};


class Iwindow{
public:
  virtual ~Iwindow() = default;
  virtual int drawScreen() = 0; //pure virtual
  virtual void handleRecv(char* msgBuf, size_t n) = 0;
  virtual void passMsg(bool loopback, char* msgBuf, size_t n) = 0;
  //virtual int updateFromRecv(char* buffer) = 0;
  bool cursorOnWindow(int y, int x);
  bool checkIfFit(const int desX, const int max_X);
  int getHeight() {return drawLoc.y + drawLoc.reqY + 1;};
  int getWidth() {return drawLoc.x + drawLoc.reqX + 1;};
  int moveWin(const int newY, const int newX);
  bool checkIdNameMatch(winNames _name,uint8_t _id);

protected:
  ScreenLoc drawLoc;
  const winNames name;
  const uint8_t id; //max 256 of the same window at once
  conn_t& conn;
  Iwindow(int yReq,int xReq,winNames _name,uint8_t _id, conn_t& _Conn) //maybe move
    :drawLoc(yReq, xReq)
    ,name{_name}
    ,id{_id}
    ,conn{_Conn} {};
};

class MainUI : public Iwindow { 
public:
  MainUI(uint8_t _id, conn_t& _Conn);
  //using state method design pattern for drawing
  //draws win/losses of players, connection screen and new window bar
  int drawScreen() override;
  void handleRecv(char* msgBuf, size_t n) override;
  void passMsg(bool loopback, char* msgBuf, size_t n) override;

private:
  enum class connStates {
    unconnected,
    waitingConn,
    connected,
    vsAI,
  };
  connStates state{connStates::unconnected};
  int attemptConnect();
  int wins{0};
  int losses{0};
  void drawInput();
  std::array<char, 20> inputStr;
};

class Tttgame : public Iwindow {
public:
  Tttgame(uint8_t _id, conn_t& _Conn);
  int drawScreen() override;
  void handleRecv(char* msgBuf, size_t n) override;
  void passMsg(bool loopback, char* msgBuf, size_t n) override;
  //int updateFromRecv(char* buffer) override { return 0; };//temp
  /*
   s: 0-8, where 0 is top left
  */
  //~Tttgame(); 
private:
  void aiPlay();
  std::array<char, 9> boardSpots;
  char lineChar;
  const int sqrSize{4}; //temp 4
  int addToSpot(const uint8_t spot, const char c);
};


#endif  //TICTACTOE
