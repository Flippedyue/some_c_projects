//
//  Point.cpp
//  FluidSimulation
//
//  Created by LeFlacon on 2019/12/23.
//  Copyright © 2019 LeFlacon. All rights reserved.
//

#include "Point.hpp"

#define MAX_POINT_NUM 1000

PointBuffer::PointBuffer(){
    pointbuffer_p=0;
    pointbuffer_cnt=0;
    pointbuffer_capacity=0;
}
PointBuffer::~PointBuffer(){
    delete[] pointbuffer_p;
    pointbuffer_p=nullptr;
}
void PointBuffer::resetBuffer(unsigned int cp){
    pointbuffer_capacity=cp;
    if(pointbuffer_p!=0){
        delete [] pointbuffer_p;
        pointbuffer_p=nullptr;
    }
    if(pointbuffer_capacity>0)pointbuffer_p=new Point[pointbuffer_capacity]();
    pointbuffer_cnt=0;
}
Point* PointBuffer::addPointReuse(){
    // 若粒子数量超过设置容量且不大于粒子最大容量，扩充至双倍，否则随便返回最后一个
    if(pointbuffer_cnt>=pointbuffer_capacity){
        if(pointbuffer_capacity*2>MAX_POINT_NUM){
            return pointbuffer_p+pointbuffer_cnt-1;
        }
        pointbuffer_capacity*=2;
        Point* new_data=new Point[pointbuffer_capacity]();
        memcpy(new_data,pointbuffer_p,pointbuffer_cnt*sizeof(Point));
        delete [] pointbuffer_p;
        pointbuffer_p=new_data;
    }
    //新的点
    Point* point=pointbuffer_p+pointbuffer_cnt++;
    point->pos=glm::vec3(0);
    point->density=0;
    point->pressure=0;
    point->acceleration=glm::vec3(0);
    point->velocity=glm::vec3(0);
    point->final_velocity=glm::vec3(0);
    point->next=0;
    return point;
}

