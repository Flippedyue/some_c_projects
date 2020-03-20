#ifndef Point_h
#define Point_h

#include <memory.h>
#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <vector>
    
#define ELEM_MAX 4096*10
    
// 粒子结构体
struct Point{
    glm::vec3 pos;//位置(x,y,z)
    float density;//密度
    float pressure;//压力
    glm::vec3 acceleration;//加速度
    glm::vec3 velocity;//速度
    glm::vec3 final_velocity;//最终速度
    int next;//下一个点的索引
};

// 粒子缓存类
class PointBuffer{
public:
    PointBuffer();
    virtual ~PointBuffer();
    void reset(unsigned int capacity);//重置粒子点缓存
    unsigned int size()const{return pointbuffer_cnt;}//返回当前粒子个数
    Point* get(unsigned int index)const{return pointbuffer_p+index;}//获取缓存中索引为index的点
    Point* addPointReuse();//添加粒子，返回新粒子
private:
    Point* pointbuffer_p;//粒子点缓存
    unsigned int pointbuffer_cnt;//计数
    unsigned int pointbuffer_capacity;//容量
};

#endif /* Point_h */
