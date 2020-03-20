#include "ftpClient.h"
bool recvFlag = false;
int send_seq = 0;
int ftpClient::fd = 0;
FILE* ftpClient::rFile = NULL;
sockaddr_in ftpClient::addr;
recvingWindow ftpClient::rwindow;
msgSeg ftpClient::sendSeg, ftpClient::recvSeg;
pthread_mutex_t ftpClient::mutex;

ftpClient::ftpClient() {
}

ftpClient::~ftpClient() {
  // 存在中间中断情况，在析构函数中统一CLOSE
    if(rFile != NULL) {
        fclose(rFile);
    }
    close(fd);
}

void ftpClient::close_connection()
{

}

void ftpClient::sendPkg(msgSeg *seg) {
    sendto(fd, seg, sizeof(msgSeg), 0, (struct sockaddr *)&addr, sizeof(*(struct sockaddr *)&addr));
}

void ftpClient::establish_socket_client()
{
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        printf("socket establish failed!\n");
        exit(-1);
    } // 客户端套接字不需要bind
    len = sizeof(addr);
    int n = 1000 * sizeof(msgSeg); // 1000个大小的tcpSeg
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)); // 设置接收缓冲区大小
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &n, sizeof(n)); // 设置发送缓冲区大小
}

void ftpClient::establish_connection(char const *address, char const *filename)
{
    int count;
    memset(&addr, 0, sizeof(struct sockaddr_in));
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = inet_addr(address); // 127.0.0.1
    addr.sin_port = htons(8888); // 向8888端口发送H包  8888端口用于收到connection信息

    // 第一个H包的发送  打招呼：分出新的端口号
    sendSeg.seq = 0;
    sendSeg.sign = 'H';
    sendSeg.dataSize = 0;
    cout << "client send H pkg" << endl;
    sendPkg(&sendSeg);

    while (true)
    {
        count = recv(fd, &recvSeg, sizeof(msgSeg), 0);
        // 需要接收新的端口号
        if (count == -1)
        {
            cout << "ERROR: recv error" << endl;
            exit(-1);
        }
        if (recvSeg.sign == 'H') // 收到从服务器响应过来的H信息，发来新的端口号，以后使用该端口号进行信息传递
        {
            addr.sin_port = htons(recvSeg.seq); // addr => 真实port  htons
            cout << "recv H new port " << recvSeg.seq << endl; 
            break;
        }
    }

    // 第一个S包的发送 与新的端口号建立连接
    sendSeg.seq = 0;
    sendSeg.sign = 'S';
    sendSeg.dataSize = 0;
    sendPkg(&sendSeg);
    cout << "client send 1th S pkg" << endl; // 已成功发送第一个S包，卡在了服务器发送第二个S包处
    
    
    while(true) {
        sleep(1);
        sendPkg(&sendSeg);
        count = recvfrom(fd, &recvSeg, sizeof(msgSeg), 0, (struct sockaddr *)&addr, &len);
        if(count == -1) {
            cout << "ERROR: recv error" << endl;
            exit(-1);
        }
        if(recvSeg.ack == sendSeg.seq + 1 && recvSeg.sign == 'S') { // 
            // printf("接收到了s包\n");
            break;
        }
    }

    // 模式包发送
    sendSeg.seq = recvSeg.ack;
    sendSeg.ack = recvSeg.seq + 1;
    sendSeg.sign = 'G'; // get file
    sendSeg.dataSize = strlen(filename);
    strncpy(sendSeg.buffer, filename, strlen(filename));
    sendPkg(&sendSeg);
    cout << "get file " << filename << endl;
}

void ftpClient::fileRecvProgram(char const* filename) {
    string str = RECEIVE_PACKET + filename;
    rFile = fopen(str.c_str(), "wb");
    if(rFile == NULL) {
        cout << "File open failed" << endl;
        exit(-1);
    }
    getFile();
}

void ftpClient::getFile()
{
    rwindow.emptyPos = 0;
    rwindow.recvBase = recvSeg.ack;
    while (recvFinishFlag != true) 
    {
        socketFileRecv();
    }
    cout << "file recv complete!\n";
    writeData();
}

void ftpClient::socketFileRecv() {
    int count = recv(fd, &recvSeg, sizeof(struct msgSeg), 0); // 客户端这边采取的是不指定ip、端口号的recv
    // 为什么不是recvfrom ? 
    if(count == -1) {
        cout << "ERROR: recv error!\n";
        exit(-1);
    }
    cout << "recv pkg : seq " << recvSeg.seq << " dataSize " << recvSeg.dataSize << endl; 
   
    if(recvSeg.sign == 'F') {
        recvFinishFlag = true;
        cout << "recv last pkg\n";
    }

    recvData();
}

void ftpClient::recvData()
{
    if (recvSeg.dataSize != BUFFER_SIZE && recvSeg.sign != 'F')
    {
        cout << "recv incomplete pkg, drop!\n";
        return;
    }
    if (rwindow.recvBase + rwindow.emptyPos == recvSeg.seq) // 是最靠前想要收到的数据包
    {
        strncpy(rwindow.window[rwindow.emptyPos].buffer, recvSeg.buffer, recvSeg.dataSize);
        rwindow.window[rwindow.emptyPos].dataSize = recvSeg.dataSize;
        rwindow.isRecv[rwindow.emptyPos] = true;
        while (rwindow.isRecv[rwindow.emptyPos] && rwindow.emptyPos < (RECEIVINGWINDOW_SIZE - 1))
        {
            rwindow.emptyPos++;
        }
        if (rwindow.emptyPos == RECEIVINGWINDOW_SIZE - 1)
        {
            cout << "write data" << endl;
            writeData();
        }
    } 
    else if ((rwindow.recvBase + rwindow.emptyPos < recvSeg.seq) && (rwindow.recvBase + RECEIVINGWINDOW_SIZE > recvSeg.seq)) // 乱序收到数据包
    {
        int misorder = recvSeg.seq - rwindow.recvBase;
        strncpy(rwindow.window[misorder].buffer, recvSeg.buffer, recvSeg.dataSize);
        rwindow.window[misorder].dataSize = recvSeg.dataSize;
        rwindow.isRecv[misorder] = true;
    }
    else if (rwindow.recvBase + rwindow.emptyPos > recvSeg.seq)
    {
        cout << "recv duplicate pkg!\n";
    }
    socketFileAckSend();
}

void ftpClient::writeData()
{
    assert(rFile != NULL);
    int i = 0;
    while (i < rwindow.emptyPos)
    {
        fwrite(rwindow.window[i].buffer, sizeof(char), rwindow.window[i].dataSize, rFile);
        i++;
    }
    for (int i = 0; i < rwindow.emptyPos; i++)
    {
        rwindow.isRecv[i] = false;
    }
    rwindow.recvBase = rwindow.recvBase + RECEIVINGWINDOW_SIZE - 1;
    rwindow.emptyPos = 0;
}

void ftpClient::socketFileAckSend() {
    sendSeg.seq = recvSeg.ack;
    sendSeg.ack = rwindow.recvBase + rwindow.emptyPos;
    printf("send ack : ack %d\n", sendSeg.ack);
    sendPkg(&sendSeg);
}