#ifndef TICTACTOE
#define TICTACTOE

#include <array>
#include <atomic>
#include <csignal>
#include <cstddef>
#include <cstdint>
#include <ncurses.h>
#include <sys/poll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <memory>
#include <thread>
#include <unistd.h>
#include <vector>
#include <mutex>

#define MAXCONNECTIONS_ 5
#define TIMEOUT_ 5000
#define BUFFSIZE_ 256

class Connector;
class Iwindow;

typedef std::unique_ptr<Connector> conn_t;
typedef std::vector<std::unique_ptr<Iwindow>> winVec_t;

static volatile sig_atomic_t mainLoopRun{1};
inline std::mutex drawMtx;
inline int tmpEOMdet = 0;

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

/* Converts recieved bytes from recv() for use in dataSize input to Message struct*/
ssize_t getDataSize(const ssize_t retnBytes);
enum msgActions {
endThread = 0,    //to self, end thread
newData = 1,      //new data from a client or server
newDataLocal = 2, //new data, send locally too
eBadMsg = 3,      //error, newData msg couldnt be handled
requestInst = 4,  //request the state of the instance window
};
/*
 data is sent in the format:
 0:  [uint8_t action]
 1:  [uint8_t window ID]
 2:  [std::array<uint8_t or char> window specific data]
 */
template <typename T, size_t dataSize>
struct Message{
  uint8_t action;
  uint8_t winID;
  std::array<T, dataSize> data;
};

template <typename T>
class WinMessage { //FIX: remove this class, added to Connector
protected:
  struct SpecMessage{
    msgActions action;
    uint8_t winID;
    std::vector<T> data;
  };
//  SpecMessage recvMsg; //remove for std::unique_ptr ?
//  SpecMessage sendMsg;
public:
  ~WinMessage() = default;
  virtual void deSerialize(void* pData, size_t dataLen); 
  /* Convert to C array and write to pipe for socket sending, 
     override for struct or other specifc template T types */
  virtual void serializeAndWrite(SpecMessage& msgStruct, int pipeWriteFD);
};
struct msgTypes{
  template <typename T>
  struct base{
    msgActions action;
    uint8_t winID;
    std::vector<T> data;
  };
  //for use in T:
  struct pointVal{
    uint8_t x;
    uint8_t y;
    uint8_t val;
  };
};

class ABserverClient{
public:
  virtual ~ABserverClient() = default;
  const bool isInstance;
  std::atomic<bool> threadLoopRun{true};
  std::thread tThread;
  void startThread(winVec_t& _vWindows);
protected:
  virtual int acceptFunc() = 0;
  int sendConnected();
  void mainLoop(winVec_t& _vWindows);
  ssize_t bufRecvn;
  std::array<uint8_t,BUFFSIZE_> recvBuffer;
  const int maxConnections;
  const nfds_t conFDsn_max;
  //conFDs[0] is always the pipe read FD
  nfds_t conFDsn_cur;
  struct pollfd* conFDs;
  const int instPort{57384};
  int selfSocket;
  sockaddr_in serverAddress;
  int startSocket();
  ABserverClient(bool isInstance_,int maxConns_, int pipeReadFD);
};

class Connector{
public:
  template <typename T>
  int sendMsgStruct(bool sendLocal, msgTypes::base<T>& sendData);
  virtual void deSerialize(void* pData,const size_t dataLen) = 0;
  /* send data from one window to another locally */
  Connector(winVec_t& _vWindows);
  ~Connector();
  int endThread(bool restartT);
  /* send data to clients or server instance through thread */
private:
  int pipeFD[2]; // [0] = read end FD, [1] = write end FD
  int startClientorInstance(bool isClient, bool replacePtr);
  winVec_t& vWindows;
  std::unique_ptr<ABserverClient> serverClient;

  class ClientSocket : public ABserverClient {
  public:
    ClientSocket(int pipeReadFD, winVec_t& _vWindows);
    ~ClientSocket();
  private:
    int acceptFunc() override;
  };
  class InstanceSocket : public ABserverClient {
  public:
    InstanceSocket(int maxConns_, int pipeReadFD, winVec_t& _vWindows);
    ~InstanceSocket();
  private:
    int acceptFunc() override;
  };
};


class Iwindow{
public:
  virtual ~Iwindow() = default;
  virtual int drawScreen() = 0; //pure virtual
  virtual void handleRecv(void* msgBuf, size_t n) = 0; //cant use Message struct here because no template virtual :(
  //virtual int updateFromRecv(char* buffer) = 0;
  bool cursorOnWindow(int y, int x);
  bool checkIfFit(const int desX, const int max_X);
  int getHeight() {return drawLoc.y + drawLoc.reqY + 1;};
  int getWidth() {return drawLoc.x + drawLoc.reqX + 1;};
  int moveWin(const int newY, const int newX);
  bool checkIdNameMatch(winNames _name,uint8_t _id);
  const uint8_t id; //max 256 of the same window at once

protected:
  ScreenLoc drawLoc;
  const winNames name;
  conn_t& conn;
  Iwindow(int yReq,int xReq,winNames _name,uint8_t _id, conn_t& _Conn) //maybe move
    :drawLoc(yReq, xReq)
    ,id{_id}
    ,name{_name}
    ,conn{_Conn} {};
};

class MainUI : public Iwindow { 
public:
  MainUI(uint8_t _id, conn_t& _Conn);
  //using state method design pattern for drawing
  //draws win/losses of players, connection screen and new window bar
  int drawScreen() override;
  void handleRecv(void* msgBuf, size_t n) override;

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
  void handleRecv(void* msgBuf, size_t n) override;
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
