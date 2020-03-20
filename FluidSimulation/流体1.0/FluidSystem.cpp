//
//  FluidSystem.cpp
//  FluidSimulation
//
//  Created by LeFlacon on 2019/12/23.
//  Copyright © 2019 LeFlacon. All rights reserved.
//

#include "FluidSystem.hpp"
#include <math.h>
#include <glm/exponential.hpp>
#include <glm/geometric.hpp>

FluidSystem::FluidSystem(){
    fluid_scale=0.004f;
    fluid_viscosity=1.0f;
    fluid_density=1000.f;
    fluid_point_mass=0.0006f;
    gas_const_k=1.0f;
    smooth_radius=0.01f;
    fluid_point_radius=0.0;
    m_rexSize[0]=0;
    m_rexSize[1]=0;
    m_rexSize[2]=0;
    boundary_stiffness=10000.f;
    boundary_dampening=256;
    fluid_limit_speed=200;
    // 光滑核函数用Poly6函数，以下三个对应密度/压力/粘度公式前面的部分常数
    kernel_poly6=315.0f/(64.0f*3.141592f*pow(smooth_radius,9));
    kernel_spiky=-45.0f/(3.141592f*pow(smooth_radius,6));
    kernel_viscosity=45.0f/(3.141592f*pow(smooth_radius,6));
}
FluidSystem::~FluidSystem(){
}
void FluidSystem::createFluidPoint(const FluidBox &fluidBox,float spacing){
    for(float z=fluidBox.max.z;z>=fluidBox.min.z;z-=spacing){
        for(float y=fluidBox.min.y;y<=fluidBox.max.y;y+=spacing){
            for(float x=fluidBox.min.x;x<=fluidBox.max.x;x+=spacing){
                Point* p=fluid_point_buffer.addPointReuse();
                p->pos=glm::vec3(x,y,z);
            }
        }
    }
}
void FluidSystem::initFluidSystem(unsigned short maxPointCounts,const FluidBox &wallBox,const FluidBox &initFluidBox,const glm::vec3 &gravity){
    fluid_point_buffer.resetBuffer(maxPointCounts);
    fluid_wall_box=wallBox;
    vec_gravity=gravity;
    fluid_point_radius=pow(fluid_point_mass/fluid_density,1.0/3.0);
    createFluidPoint(initFluidBox,fluid_point_radius/fluid_scale);
    //第三个参数：单元格大小一般是光滑核半径的两倍
    fluid_grid.init(wallBox,fluid_scale,smooth_radius*2.0,1.0,m_rexSize);
    fluid_point_pos=std::vector<float>(3*fluid_point_buffer.size(),0);
}
unsigned int FluidSystem::getPointSize()const{
    return sizeof(Point);
}
unsigned int FluidSystem::getPointCnt()const{
    return fluid_point_buffer.size();
}
const Point* FluidSystem::getPointBuf()const{
    return fluid_point_buffer.get(0);
}
void FluidSystem::computerPressure(){
    /*
    密度公式：
    \begin{equation} 
    \rho(r_i)=m\frac{315}{64\pi h^9}\sum_j{\left(h^2-|\vec{r_i}-\vec{r_j}|^2\right)^3}\tag{3.7} 
    \end{equation}
    压力公式：
    \begin{equation} 
    \vec{a_i}^{pressure}=-\frac{\nabla p(\vec{r_i})}{\rho_i}=m\frac{45}{\pi h^6}\sum_j{\left(\frac{p_i+p_j}{2\rho_i\rho_j}(h-r)^2\frac{\vec{r_i}-\vec{r_j}}{r}\right)}\\ \text{with}\ r=|\vec{r_i}-\vec{r_j}|\tag{3.13} 
    \end{equation}
    */
    float smooth_radius_pow2=smooth_radius*smooth_radius;
    fluid_neighbor_table.reset(fluid_point_buffer.size());
    // 循环对每一个粒子计算邻域
    for(unsigned int i=0;i<fluid_point_buffer.size();i++){
        Point* pi=fluid_point_buffer.get(i);
        float sum=0.0;
        fluid_neighbor_table.pointInitCur(i);
        // 获取八个邻域网格
        int gridCell[8];
        fluid_grid.getNeighborGrids(pi->pos,smooth_radius/fluid_scale,gridCell);
        for(int cell=0;cell<8;cell++){
            // 没有点
            if(gridCell[cell]==-1)continue;
            // 获取头节点
            int pndx=fluid_grid.getGridPoints(gridCell[cell]);
            while(pndx!=-1) {
                Point* pj=fluid_point_buffer.get(pndx);
                if(pj==pi)sum+=pow(smooth_radius_pow2,3.0);
                else{
                    glm::vec3 pi_pj=(pi->pos-pj->pos)*fluid_scale;
                    float r2=pi_pj.x*pi_pj.x+pi_pj.y*pi_pj.y+pi_pj.z*pi_pj.z;
                    if(smooth_radius_pow2>r2){
                        float smooth_radius_pow2_r2=smooth_radius_pow2-r2;
                        sum+=pow(smooth_radius_pow2_r2,3.0);
                        if(!fluid_neighbor_table.addNeighbor(pndx,glm::sqrt(r2)))goto NEIGHBOR_FULL;
                    }
                }
                pndx=pj->next;
            }
        }
        NEIGHBOR_FULL:
            // 当前节点递交邻接表，更新密度，更新压力
            fluid_neighbor_table.commitToTable();
            pi->density=kernel_poly6*fluid_point_mass*sum;
            pi->pressure=(pi->density-fluid_density)*gas_const_k;
    }
}
void FluidSystem::computerAcceleration(){
    /*
    加速度公式：
    \begin{equation} 
    \vec{a}(r_i)=\vec{g}+m\frac{45}{\pi h^6}\sum_j\left(\frac{p_i+p_j}{2\rho_i\rho_j}(h-r)^2\frac{\vec{r_i}-\vec{r_j}}{r}\right)+m\mu\frac{45}{\pi h^6}\sum_j\frac{\vec{u_j}-\vec{u_i}}{\rho_i\rho_j}(h-r)\\ \text{with}\ r=|\vec{r_i}-\vec{r_j}| \tag{3.19} 
    \end{equation}
    */
    float smooth_radius_pow2=smooth_radius*smooth_radius;
    for(unsigned int i=0;i<fluid_point_buffer.size();i++){
        Point* pi=fluid_point_buffer.get(i);
        glm::vec3 acceleration_sum=glm::vec3(0.0);
        int extra_neighbor_cnt=fluid_neighbor_table.getNeighborCnt(i);
        for(unsigned int j=0;j<extra_neighbor_cnt;j++){
            unsigned short neighborIndex;
            float r;
            fluid_neighbor_table.getNeighborInfo(i,j,neighborIndex,r);
            Point* pj=fluid_point_buffer.get(neighborIndex);
            glm::vec3 ri_rj=(pi->pos-pj->pos)*fluid_scale;
            float h_r=smooth_radius-r;
            float pterm=-fluid_point_mass*kernel_spiky*h_r*h_r*
            (pi->pressure+pj->pressure)/(2.f*pi->density*pj->density);
            acceleration_sum+=ri_rj*pterm/r;
            float vterm=kernel_viscosity*fluid_viscosity*h_r*
            fluid_point_mass/(pi->density*pj->density);
            acceleration_sum+=(pj->final_velocity-pi->final_velocity)*vterm;
        }
        pi->acceleration=acceleration_sum;
    }
}
void FluidSystem::computerPos(){
    float time_interval=0.003;//计算时间间隔
    float limit_speed_pow2=fluid_limit_speed*fluid_limit_speed;
    for(unsigned int i=0;i<fluid_point_buffer.size();i++){
        Point* p=fluid_point_buffer.get(i);
        glm::vec3 acceleration=p->acceleration;
        float acceleration_pow2=acceleration.x*acceleration.x+acceleration.y*acceleration.y+acceleration.z*acceleration.z;
        // 速度限制
        if(acceleration_pow2>limit_speed_pow2)acceleration*=fluid_limit_speed/glm::sqrt(acceleration_pow2);
        float diff;
        // Z方向边界
        diff=1*fluid_scale-(p->pos.z-fluid_wall_box.min.z)*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(0,0,1.0);
            float adj=boundary_stiffness*diff-boundary_dampening*glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        diff=1*fluid_scale-(fluid_wall_box.max.z-p->pos.z)*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(0,0,-1.0);
            float adj=boundary_stiffness*diff-boundary_dampening *glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        //X方向边界
        diff=1*fluid_scale-(p->pos.x-fluid_wall_box.min.x)*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(1.0,0,0);
            float adj=boundary_stiffness*diff-boundary_dampening*glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        diff=1*fluid_scale-(fluid_wall_box.max.x-p->pos.x)*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(-1.0,0,0);
            float adj=boundary_stiffness*diff-boundary_dampening*glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        //Y方向边界
        diff=1*fluid_scale-(p->pos.y-fluid_wall_box.min.y )*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(0,1.0,0);
            float adj=boundary_stiffness*diff-boundary_dampening*glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        diff=1*fluid_scale-( fluid_wall_box.max.y-p->pos.y )*fluid_scale;
        if(diff>0.0f){
            glm::vec3 norm(0,-1.0,0);
            float adj=boundary_stiffness*diff-boundary_dampening*glm::dot(norm,p->final_velocity);
            acceleration.x+=adj*norm.x;
            acceleration.y+=adj*norm.y;
            acceleration.z+=adj*norm.z;
        }
        acceleration+=vec_gravity;//重力
        // 计算位置
        glm::vec3 newv=p->velocity+acceleration*time_interval;//v(t+1/2)=v(t-1/2)+at
        p->final_velocity=(p->velocity+newv)*0.5f;//v(t+1)=[v(t-1/2)+v(t+1/2)]*0.5
        p->velocity=newv;
        p->pos+=newv*time_interval/fluid_scale;//p'=p+vt
        // 位置缓存
        fluid_point_pos[3*i]=p->pos.x;
        fluid_point_pos[3*i+1]=p->pos.y;
        fluid_point_pos[3*i+2]=p->pos.z;
    }
}
void FluidSystem::tick(){
    // 每帧刷新粒子位置
    fluid_grid.insertPoints(&fluid_point_buffer);
    computerPressure();
    computerAcceleration();
    computerPos();
}