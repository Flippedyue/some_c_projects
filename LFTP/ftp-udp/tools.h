#ifndef TOOLS_H
#define TOOLS_H

#include <iostream>
#include <vector>
#include <sys/wait.h>
#include <sys/socket.h>
#include <sys/time.h>
#include <cstdio>
#include <cstring>
#include <cstdlib>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <time.h>
#include <assert.h>
#include <signal.h>
#include <stdbool.h>
#include <cmath>
#include <pthread.h>

#define BUFFER_SIZE 1024
#define SENDINGWINDOW_SIZE 6
#define RECEIVINGWINDOW_SIZE 6
using namespace std;

static string RECEIVE_PACKET = "recv_files/";

struct msgSeg
{
  int seq;
  int ack;
  int dataSize;
  char sign;
  char buffer[BUFFER_SIZE]; //后期可改进的地方->非固定大小

  msgSeg() {
    seq = 0;
    ack = 0;
    dataSize = 0;
    sign = 0;
  }
};

struct sendingWindow
{
  int head;
  int tail;
  int sendBase;
  int nextseqnum;
  struct msgSeg window[SENDINGWINDOW_SIZE];
  sendingWindow()
  {
    head = 0;
    tail = 0;
    sendBase = 0;
    nextseqnum = 0;
  }
};

struct recvingWindow
{
  int recvBase;
  int emptyPos; //未被使用的区域
  struct msgSeg window[RECEIVINGWINDOW_SIZE];
  bool isRecv[RECEIVINGWINDOW_SIZE];
  recvingWindow() {
    recvBase = 0;
    emptyPos = 0;
    for (int i = 0; i < RECEIVINGWINDOW_SIZE; i++)
    {
      isRecv[i] = false;
    }
  }
};

#endif