#ifndef NeighborTable_h
#define NeighborTable_h

#include <stdio.h>
#include <string.h>

#define MAX_NEIGHBOR_CNT 80

class NeighborTable{
private:
    union PointExtraData{
        struct{
            unsigned extra_neighbor_offset:24;//偏移
            unsigned extra_neighbor_cnt:8;//个数
        };
        unsigned int neighborData;//数据
    };
    PointExtraData* point_extra_data;//额外信息
    unsigned int neighbor_point_cnt;//粒子数
    unsigned int neighbor_point_capacity;//粒子容量
    unsigned char* neighbor_point_buffer;//数据缓存
    unsigned int neighbor_buffer_size;//缓存大小
    unsigned int neighbor_buffer_offset;//缓存中的偏移
    unsigned short cur_point;//索引
    int cur_neighbor_cnt;//邻域点数量
    unsigned short cur_neighbor_index[MAX_NEIGHBOR_CNT];//邻域中点的索引
    float cur_neighbor_distance[MAX_NEIGHBOR_CNT];//邻域中点的距离
public:
    NeighborTable();
    ~NeighborTable();
    void reset(unsigned short pointCounts);//重置邻接表
    void pointInitCur(unsigned short point_index);//初始化当前点
    bool addNeighbor(unsigned short point_index,float distance);//加入符合邻域的粒子
    void commitToTable();//递交粒子给邻接表
    void updatePointBuffer(unsigned int need_size);//扩容用子函数
    int getNeighborCnt(unsigned short point_index);//获取邻接表中的点个数
    void getNeighborInfo(unsigned short point_index,int index,unsigned short& neighborIndex,float& neighborDistance);//获取索引point_index的邻接表中第index个点的数据
};

#endif /* NeighborTable_h */
