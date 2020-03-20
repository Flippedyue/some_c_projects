#ifndef FTPCLIENT_H
#define FTPCLIENT_H
#include "tools.h"

class ftpClient {
private:
    static bool recvFlag;
    static int idleCounter;
    bool recvFinishFlag;
    socklen_t len;
public:
    static int fd;
    static FILE *rFile;
    static sockaddr_in addr;
    static recvingWindow rwindow;
    static msgSeg sendSeg, recvSeg;
    static pthread_mutex_t mutex;
    static void sendPkg(msgSeg *seg);
    ftpClient();
    ~ftpClient();
    void establish_socket_client();
    void socketFileRecv();
    void establish_connection(char const *address, char const *filename);
    void socketFileAckSend();
    void getFile();
    void close_connection();
    void recvData();
    void writeData();
    void messagePrint();
    void fileRecvProgram(char const* filename);
};
#endif