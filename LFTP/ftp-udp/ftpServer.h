#ifndef FTPSERVER_H
#define FTPSERVER_H
#include "tools.h"
extern int send_seq;
extern bool begin;
class ftpServer {
private:
    // static bool recvFlag;
    static int idleCounter;
    static int TimeoutInterval;
    bool sendFinishFlag, recvFinishFlag;
    socklen_t len;
public:
    static int fd;
    static FILE *sFile;
    static sockaddr_in addr;
    static sendingWindow swindow;
    static msgSeg sendSeg, recvSeg;
    static pthread_mutex_t mutex;
    static void startTimer(int t);
    static void sendPkg(msgSeg *seg);
    static void timeoutHandle(int signum);
    ftpServer();
    ~ftpServer();
    void accept(char *filename);
    void establish_socket_server(int port);
    void sendFile();
    void socketFileSend();
    void socketFileAckRecv();
    void close_connection();
    void makeNextPkt(msgSeg *seg);
    void fileSendProgram(char const* filename);
};

#endif