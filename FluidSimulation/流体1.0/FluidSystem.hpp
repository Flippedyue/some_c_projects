//
//  FluidSystem.hpp
//  FluidSimulation
//
//  Created by LeFlacon on 2019/12/23.
//  Copyright © 2019 LeFlacon. All rights reserved.
//

#ifndef FluidSystem_hpp
#define FluidSystem_hpp

#include <stdio.h>
#include "Box.hpp"
#include "NeighborTable.hpp"

class FluidSystem{
private:
    //三个核心数据成员
    PointBuffer fluid_point_buffer;
    FluidGrid fluid_grid;
    NeighborTable fluid_neighbor_table;
    std::vector<float>fluid_point_pos;//点位置缓存
    //SPH光滑核相关
    float kernel_poly6;
    float kernel_spiky;
    float kernel_viscosity;
    int m_rexSize[3];//网格尺寸
    float fluid_point_radius;//半径
    float fluid_scale;//尺寸单位
    float fluid_viscosity;//粘性
    float fluid_density;//静态密度
    float fluid_point_mass;//质量
    float smooth_radius;//光滑核半径
    float gas_const_k;//气体常量k
    float boundary_stiffness;//边界刚性
    float boundary_dampening;//边界阻尼
    float fluid_limit_speed;//速度限制
    glm::vec3 vec_gravity;//重力矢量
    FluidBox fluid_wall_box;//立方体
    void initFluidSystem(unsigned short maxPointCounts,const FluidBox& wallBox,const FluidBox& initFluidBox,const glm::vec3& gravity);//初始化
    void computerPressure();//计算邻接表，密度，压力
    void computerAcceleration();//计算加速度
    void computerPos();//计算新位置
    void createFluidPoint(const FluidBox& fluidBox,float spacing);//初始化流体粒子块
public:
    FluidSystem();
    void init(unsigned short maxPointCounts,
              const glm::vec3& wallBox_min,const glm::vec3& wallBox_max,
              const glm::vec3& initFluidBox_min,const glm::vec3& initFluidBox_max,
              const glm::vec3& gravity){
        initFluidSystem(maxPointCounts,FluidBox(wallBox_min,wallBox_max),
              FluidBox(initFluidBox_min,initFluidBox_max),gravity);
    }    
    unsigned int getPointSize()const;//点的size
    unsigned int getPointCnt()const;//点的数量
    const Point* getPointBuf()const;//点缓存
    void tick();//逻辑桢
    ~FluidSystem();
};

#endif /* FluidSystem_hpp */
