#include "ftpServer.h"

struct connection_record
{
    struct sockaddr_in client_addr;
    int port;
    pid_t pid;
    
};
vector<connection_record> conn_records;

void SIGCHLD_Handle(int signo) {
    int status, t;
    while ((t = waitpid(-1, &status, WNOHANG)) > 0) 
    {
        // 删去记录表的该条记录
        for (int i = 0; i < conn_records.size(); i++) 
        {
            if (t == conn_records[i].pid) 
            {
                cout << "exit process pid: " << t << ", client ip: " << ntohl(conn_records[i].client_addr.sin_addr.s_addr)
                << ", server port: " << conn_records[i].port << endl;
                conn_records.erase(conn_records.begin() + i);
                break;
            }
        }
    }  
}

int getClientPort(sockaddr_in client_addr) {
    for (size_t i = 0; i < conn_records.size(); i++) 
    {
        if (conn_records[i].client_addr.sin_addr.s_addr == client_addr.sin_addr.s_addr && conn_records[i].client_addr.sin_port == client_addr.sin_port && conn_records[i].client_addr.sin_family == client_addr.sin_family) 
        {
            return conn_records[i].port;
        }
    }
    return 0;
}

int getFreePort() {
    struct sockaddr_in addr;
    socklen_t len = sizeof(addr);
    int port = 0;
    int fd = socket(AF_INET, SOCK_DGRAM, 0); // 创建新的套接字,为了返回空闲端口号
    if (fd < 0)
    {
        cout << "create new server socket failed!" << endl;
        exit(-1);
    }

    memset(&addr, 0, sizeof(addr));
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = 0;
    addr.sin_family = AF_INET;
    if (bind(fd, (struct sockaddr *)&addr, sizeof(addr)) < 0)
    {
        cout << "socket bind addr failed!" << endl;
        close(fd);
    }
    if (getsockname(fd, (struct sockaddr *)&addr, &len) == 0)
    {
        // 用于获取一个套接字的名字
        port = ntohs(addr.sin_port);
        cout << "get free server port: " << port << endl;
    } // 作用是获取本地空闲端口
    close(fd);
    return port;
}


int main(int argc, char const *argv[])
{
    int sfd, port, count;
    srand((unsigned)time(NULL));
    signal(SIGCHLD, SIGCHLD_Handle); // 处理僵尸进程，捕捉信号

    struct sockaddr_in server_addr, client_addr;
    socklen_t len = sizeof(client_addr);
    msgSeg recvSeg, sendSeg;
    int fd = socket(AF_INET, SOCK_DGRAM, 0); // 用于连接的套接字
    if (fd < 0)
    {
        cout << "create server socket failed!" << endl;
        exit(-1);
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(8888); // 用来监听的端口

    if (bind(fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        cout << "server bind failed!" << endl;
        close(fd);
        exit(-1);
    }

    while (true)
    {
        count = recvfrom(fd, &recvSeg, sizeof(recvSeg), 0, (struct sockaddr *)&client_addr, &len);
        if (count == -1) // 接收失败
        {
            cout << "ERROR: recv error!\n";
            exit(-1);
        }
        cout << "recv pkg\n";

        if (recvSeg.sign == 'H')
        {
            cout << "recv Hi pkg\n";
            port = getClientPort(client_addr); 
            // 看这个客户端是否已经在用户列表中：如果已经与该服务器建立连接，则返回该服务器与其通讯的端口号；否则，返回0
            // 已经在用户列表中的情况发生在：客户端已经发送Hi请求，并且服务器也给它分配了通讯的端口号，但它没有收到相应的回复。
            if (port != 0) // 不为0，说明已经建立连接
            {
                cout << "Is acked client\n";
                sendSeg.seq = port;
                sendSeg.ack = port;
                sendSeg.dataSize = 0;
                sendSeg.sign = 'H';
                sendto(fd, &sendSeg, sizeof(msgSeg), 0, (sockaddr *)&client_addr, sizeof(*(sockaddr *)&client_addr));
                cout << "resend port: " << port << endl;
            }
            else
            {
                port = getFreePort();
                sendSeg.seq = port;
                sendSeg.ack = port;
                sendSeg.dataSize = 0;
                sendSeg.sign = 'H';
                sendto(fd, &sendSeg, sizeof(msgSeg), 0, (sockaddr *)&client_addr, sizeof(*(sockaddr *)&client_addr));
                pid_t pid = fork();
                if (pid < 0) // 创建子进程失败
                {
                    perror("fork error");
                    exit(EXIT_FAILURE);
                }
                else if (pid == 0)
                {
                    cout << "create new process, port - " << port << endl;
                    char program[30], new_port[10], client_port[10], client_ip[30];
                    sprintf(program, "%s", "/.server");
                    sprintf(new_port, "%d", port);
                    sprintf(client_port, "%d", ntohs(client_addr.sin_port));
                    inet_ntop(AF_INET, &(client_addr.sin_addr), client_ip, INET_ADDRSTRLEN);
                    char *argv[5], *envp[1];
                    argv[0] = program;
                    argv[1] = new_port;
                    argv[2] = client_port;
                    argv[3] = client_ip;
                    argv[4] = NULL;
                    envp[0] = NULL;
                    execve("./server", argv, envp);
                }
                else // 父进程执行
                {
                    connection_record temp;
                    temp.client_addr = client_addr;
                    temp.pid = pid;
                    temp.port = port;
                    conn_records.push_back(temp);
                }
            }
        }
    }
    close(fd);
    return 0;
}
