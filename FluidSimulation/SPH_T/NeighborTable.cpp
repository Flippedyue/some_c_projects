#include "NeighborTable.h"

NeighborTable::NeighborTable(){
    point_extra_data=0;
    neighbor_point_cnt=0;
    neighbor_point_buffer=0;
    neighbor_buffer_size=0;
    neighbor_buffer_offset=0;
    cur_point=0;
    cur_neighbor_cnt=0;
}
NeighborTable::~NeighborTable(){
    if(point_extra_data){
        delete [] point_extra_data;
        point_extra_data=nullptr;
    }
    if(neighbor_point_buffer){
        delete [] neighbor_point_buffer;
        neighbor_point_buffer=nullptr;
    }
}
void NeighborTable::reset(unsigned short pointCounts){
    int a=sizeof(PointExtraData);
    if(pointCounts>neighbor_point_capacity){
        if(point_extra_data){
            delete [] point_extra_data;
            point_extra_data=nullptr;
        }
        point_extra_data=new PointExtraData[a*pointCounts]();
        neighbor_point_capacity=neighbor_point_cnt;
    }
    neighbor_point_cnt=pointCounts;
    memset(point_extra_data,0,a*neighbor_point_capacity);
    neighbor_buffer_offset=0;
}
void NeighborTable::pointInitCur(unsigned short point_index){
    cur_point=point_index;
    cur_neighbor_cnt=0;
}
bool NeighborTable::addNeighbor(unsigned short point_index,float distance){
    if(cur_neighbor_cnt>=MAX_NEIGHBOR_CNT)return false;
    cur_neighbor_index[cur_neighbor_cnt]=point_index;
    cur_neighbor_distance[cur_neighbor_cnt++]=distance;
    return true;
}
void NeighborTable::commitToTable(){
    if(cur_neighbor_cnt==0)return;
    unsigned int index_size=cur_neighbor_cnt*sizeof(unsigned short);
    unsigned int distance_size=cur_neighbor_cnt*sizeof(float);
    //扩容
    if(neighbor_buffer_offset+index_size+distance_size>neighbor_buffer_size)updatePointBuffer(neighbor_buffer_offset+index_size+distance_size);
    point_extra_data[cur_point].extra_neighbor_cnt=cur_neighbor_cnt;
    point_extra_data[cur_point].extra_neighbor_offset=neighbor_buffer_offset;
    //复制到缓存中
    memcpy(neighbor_point_buffer+neighbor_buffer_offset,cur_neighbor_index,index_size);
    neighbor_buffer_offset+=index_size;
    memcpy(neighbor_point_buffer+neighbor_buffer_offset,cur_neighbor_distance,distance_size);
    neighbor_buffer_offset+=distance_size;
}
void NeighborTable::updatePointBuffer(unsigned int need_size){
    unsigned int newSize=neighbor_buffer_size>0?neighbor_buffer_size:1;
    while (newSize<need_size)newSize*=2;
    if(newSize<2024)newSize=1024;
    unsigned char* newBuf=new unsigned char[newSize]();
    if(neighbor_point_buffer){
        memcpy(newBuf,neighbor_point_buffer,neighbor_buffer_size);
        delete [] neighbor_point_buffer;
    }
    neighbor_point_buffer=newBuf;
    neighbor_buffer_size=newSize;
}
int NeighborTable::getNeighborCnt(unsigned short point_index){
    return point_extra_data[point_index].extra_neighbor_cnt;
}
void NeighborTable::getNeighborInfo(unsigned short point_index,int index,unsigned short &neighborIndex,float &neighborDistance){
    PointExtraData neighData=point_extra_data[point_index];
    unsigned short* indexBuf=(unsigned short*)(neighbor_point_buffer+neighData.extra_neighbor_offset);
    float* distanceBuf=(float*)(neighbor_point_buffer+neighData.extra_neighbor_offset+sizeof(unsigned short)*neighData.extra_neighbor_cnt);
    neighborIndex=indexBuf[index];
    neighborDistance=distanceBuf[index];
}

