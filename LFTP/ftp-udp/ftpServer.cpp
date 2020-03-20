#include "ftpServer.h"
bool recvFlag = false;
bool first = true;
int send_seq = 0;
int ftpServer::fd = 0;
FILE* ftpServer::sFile = NULL;
sockaddr_in ftpServer::addr;
sendingWindow ftpServer::swindow;
msgSeg ftpServer::sendSeg, ftpServer::recvSeg;
int ftpServer::TimeoutInterval = 500;
pthread_mutex_t ftpServer::mutex;

ftpServer::ftpServer() {
    sFile = NULL;
    sendFinishFlag = false;
    srand((unsigned)time(NULL));
    signal(SIGALRM, ftpServer::timeoutHandle);
}

ftpServer::~ftpServer() {
    if (sFile != NULL)
    {
        fclose(sFile);
    }
    close(fd);
}

void ftpServer::close_connection()
{

}

void ftpServer::establish_socket_server(int port) {
    struct sockaddr_in server_addr;
    fd = socket(AF_INET, SOCK_DGRAM, 0);
    if (fd < 0)
    {
        cout << "create server socket failed!\n";
        exit(-1);
    }
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = inet_addr("127.0.0.1"); // htonl(INADDR_ANY); 
    server_addr.sin_port = htons(port);
    socklen_t length = sizeof(server_addr);
    if (bind(fd, (struct sockaddr *)&server_addr, length) < 0)
    {
        cout << "server socket bind failed!\n";
        close(fd);
        exit(-1);
    }
    int n = 1000 * sizeof(msgSeg);
    setsockopt(fd, SOL_SOCKET, SO_RCVBUF, &n, sizeof(n)); // 设置接收缓冲区大小
    setsockopt(fd, SOL_SOCKET, SO_SNDBUF, &n, sizeof(n)); // 设置发送缓冲区大小
    len = sizeof(addr);
    // memset(&addr, 0, sizeof(len));
}

void ftpServer::accept(char *filename) {
    // startTimer(1000 * 10);
    int round = 0;
    while(round < 2) {
        // int count = recv(fd, &recvSeg, sizeof(recvSeg), 0);
        int count = recvfrom(fd, &recvSeg, sizeof(struct msgSeg), 0, (struct sockaddr *)&addr, &len);
        recvFlag = true;
        // 不能有多个客户端同时请求连接  可能是静态成员的问题？ addr
        if(count == -1) { 
            printf("error : recv error\n");
            exit(-1);
        }
        switch(round) {
        case 0:
            if(recvSeg.sign == 'S') {
                sendSeg.seq = 0;
                sendSeg.ack = recvSeg.seq + 1; // 是收到的数据包的下一个seq
                sendSeg.sign = 'S';
                cout << "server send 2th S pkg\n";
                sendPkg(&sendSeg);          
                round += 1; // 
            }
            break;
        case 1:
            if(recvSeg.sign == 'G') { 
                strncpy(filename, recvSeg.buffer, recvSeg.dataSize);
                filename[recvSeg.dataSize] = '\0';
                swindow.sendBase = recvSeg.ack; // 客户端下一个想要收到的数据包seq
                swindow.nextseqnum = recvSeg.ack;
                round += 1;
            } 
            else if(recvSeg.sign == 'S') {
                // 向前兼容 应对之前的包丢失
                sendSeg.seq = 0;
                sendSeg.ack = recvSeg.seq + 1;
                sendSeg.sign = 'S';
                cout << "server send 2th S pkg again\n";
                sendPkg(&sendSeg);
            }
            break;
        }
    }
    recvFlag = false;
}

void ftpServer::sendPkg(msgSeg *seg) {
    sendto(fd, seg, sizeof(msgSeg), 0, (struct sockaddr *)&addr, sizeof(*(struct sockaddr *)&addr));
}

void ftpServer::fileSendProgram(char const* filename) {
    if (pthread_mutex_init(&mutex, NULL) != 0){
        printf("锁初始化失败\n");
        exit(-1);
    }
    sFile = fopen(filename, "rb");
    if(sFile == NULL) {
        printf("File open failed\n");
        exit(-1);
    }

    while (sendFinishFlag == false)
    {
        sendFile();
        socketFileAckRecv();
    }

    while (swindow.head != swindow.tail) {
        socketFileAckRecv();
    }
    printf("file send complete !\n");
    pthread_mutex_destroy(&mutex); // 消除互斥锁
}

void ftpServer::sendFile() { 
    printf("unacked Pkg : %d\n", swindow.sendBase);
    while ((swindow.tail + 1) % SENDINGWINDOW_SIZE != swindow.head && sendFinishFlag == false) {
        socketFileSend();
    } // tail+1=head => 窗口已满 此时开始接收ack
}

void ftpServer::socketFileSend() {
    if (((swindow.tail + 1) % SENDINGWINDOW_SIZE) == swindow.head) 
    {
        printf("sending window is full\n");
        return;
    }
    makeNextPkt(&(swindow.window[swindow.tail])); 
    if (swindow.tail == swindow.head) // 窗口中只有一个pkg，且未发送
    {
        // 给sendBase赋值
        swindow.sendBase = swindow.window[swindow.tail].seq;
        startTimer(TimeoutInterval);
    }
    swindow.nextseqnum = swindow.window[swindow.tail].seq + 1;
    printf("send pkg : seq %d dataSize %d sign %c\n", swindow.window[swindow.tail].seq, swindow.window[swindow.tail].dataSize, swindow.window[swindow.tail].sign);
    sendPkg(&(swindow.window[swindow.tail]));
    swindow.tail = (swindow.tail + 1) % SENDINGWINDOW_SIZE; // 向后挪一个
}

void ftpServer::makeNextPkt(struct msgSeg *seg) {
    assert(sFile != NULL);
    // 准备发送带数据的数据包而不是ack
    seg->seq = swindow.nextseqnum;
    seg->sign = 'O';
    seg->dataSize = BUFFER_SIZE;
    int num = fread(seg->buffer, sizeof(char), seg->dataSize, sFile);
    if (num != seg->dataSize)
    {
        if (ferror(sFile))
        {
            printf("error : read file failed\n");
            exit(-1);
        }
        else // if (feof(sFile))
        {
            printf("file transport complete !\n");
            sendFinishFlag = true;
            seg->dataSize = num;
            seg->sign = 'F';
        }
    }
}

void ftpServer::socketFileAckRecv() {
    int count;
    // 仍然存在未ack的数据包
    while ((swindow.head % SENDINGWINDOW_SIZE) != (swindow.tail % SENDINGWINDOW_SIZE ))
    {
        count = recvfrom(fd, &recvSeg, sizeof(struct msgSeg), 0, (struct sockaddr *)&addr, &len);
        cout << "recv ack: ack " << recvSeg.ack << endl;

        if (recvSeg.ack <= swindow.window[swindow.head].seq)
        {
            cout << "The seqs before seq " << recvSeg.ack << "have been acked" << endl;
        }
        else
        {
            swindow.head += recvSeg.ack - swindow.sendBase;
            swindow.head = swindow.head % SENDINGWINDOW_SIZE;
            swindow.sendBase = recvSeg.ack;
        }
    } // 等待接收完之前的ack再继续发
}
void ftpServer::startTimer(int t) //毫秒
{
    struct itimerval val;
    val.it_interval.tv_sec = 0;
    val.it_interval.tv_usec = 0;
    val.it_value.tv_sec = t / 1000;
    val.it_value.tv_usec = (t % 1000) * 1000;
    setitimer(ITIMER_REAL, &val, NULL);
    printf("Start timer : %d\n", t);
}

void ftpServer::timeoutHandle(int signum) {
    if (!recvFlag)
    {
        sendto(fd, &sendSeg, sizeof(sendSeg), 0, (struct sockaddr *)&addr, sizeof(*(struct sockaddr *)&addr));
    }
    if(swindow.head != swindow.tail) {
        sendto(fd, &(swindow.window[swindow.head]), sizeof(msgSeg), 0, (struct sockaddr *)&addr, sizeof(*(struct sockaddr *)&addr));
        cout << "timeout: sending seq " << swindow.window[swindow.head].seq << " again\n";
    }
    startTimer(TimeoutInterval);
};