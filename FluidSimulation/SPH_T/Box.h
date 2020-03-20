#ifndef Box_h
#define Box_h

#include "Point.h"

// 流体模拟的立方体
class FluidBox{
public:
    FluidBox();
    FluidBox(glm::vec3 aMin,glm::vec3 aMax);
    // min左下max右上
    glm::vec3 min;
    glm::vec3 max;
};

// 空间网格划分
class FluidGrid{
private:
    //空间网格
    std::vector<int> grid_points;//网格内的粒子（存的是索引）
    glm::vec3 grid_min;//网格左下
    glm::vec3 grid_max;//网格右上
    glm::ivec3 grid_standards;//网格规格x*y*z
    glm::vec3 grid_size;//整个网格空间大小
    glm::vec3 grid_offset;//网格偏移量（单位长度几个网格）
public:
    FluidGrid(){}
    ~FluidGrid(){}
    const glm::ivec3* getGridRes()const{return &grid_standards;}
    const glm::vec3* getGridMin()const{return &grid_min;}
    const glm::vec3* getGridMax()const{return &grid_max;}
    const glm::vec3* getGridSize()const{return &grid_size;}
    float getOffset(){return grid_offset.x;}
    void init(const FluidBox& box,float sim_scale,float cell_size,float border,int* rex);//网格初始化
    int getGridPoints(int gridIndex);//获取网格内粒子
    int getGridIndex(float px,float py,float pz);//获取位置(px,py,pz)对应的网格索引
    void insertPoints(PointBuffer*pointBuffer);//初始加入粒子到网格系统
    void getNeighborGrids(const glm::vec3& p,float radius,int *gridCell);//获取邻域网格
};

#endif /* Box_h */
