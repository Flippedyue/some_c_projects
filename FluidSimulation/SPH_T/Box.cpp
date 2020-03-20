#include "Box.h"
#include <math.h>

FluidBox::FluidBox(){
    min=glm::vec3(0);
    max=glm::vec3(0);
}
FluidBox::FluidBox(glm::vec3 minn,glm::vec3 maxx){
    min=minn;
    max=maxx;
}

void FluidGrid::init(const FluidBox &fluid_box,float sim_scale,float cell_size,float border,int* rex){
    // border边界大小
    // 计算网格空间左上右下及大小
    float world_cellsize=cell_size/sim_scale;
    grid_min=fluid_box.min;
    grid_min-=border;
    grid_max=fluid_box.max;
    grid_max+=border;
    grid_size=grid_max;
    grid_size-=grid_min;
    //计算网格规格
    grid_standards.x=(int)ceil(grid_size.x/world_cellsize);
    grid_standards.y=(int)ceil(grid_size.y/world_cellsize);
    grid_standards.z=(int)ceil(grid_size.z/world_cellsize);
    //根据规格，网格空间大小微调
    grid_size.x=grid_standards.x*cell_size/sim_scale;
    grid_size.y=grid_standards.y*cell_size/sim_scale;
    grid_size.z=grid_standards.z*cell_size/sim_scale;
    //计算偏移量
    grid_offset=grid_standards;
    grid_offset/=grid_size;
    int grid_total=(int)(grid_standards.x*grid_standards.y*grid_standards.z);
    grid_points.resize(grid_total);
    rex[0]=grid_standards.x*8;
    rex[1]=grid_standards.y*8;
    rex[2]=grid_standards.z*8;
}
int FluidGrid::getGridPoints(int gridIndex){
    if(gridIndex<0||gridIndex>=grid_points.size())return -1;
    return grid_points[gridIndex];
}
int FluidGrid::getGridIndex(float px,float py,float pz){
    int gx=(int)((px-grid_min.x)*grid_offset.x);
    int gy=(int)((py-grid_min.y)*grid_offset.y);
    int gz=(int)((pz-grid_min.z)*grid_offset.z);
    return (gz*grid_standards.y+gy)*grid_standards.x+gx;
}
void FluidGrid::insertPoints(PointBuffer *pointBuffer){
    // grid_points的所有索引初始化为-1
    std::fill(grid_points.begin(),grid_points.end(),-1);
    Point* p=pointBuffer->get(0);
    for(unsigned int n=0;n<pointBuffer->size();n++,p++){
        int gi=getGridIndex(p->pos.x,p->pos.y,p->pos.z);
        // 一个网格内的粒子以链表形式存储
        if(gi>=0&&gi<grid_points.size()){
            p->next=grid_points[gi];
            grid_points[gi]=n;
        }
        else p->next=-1;
    }
}
void FluidGrid::getNeighborGrids(const glm::vec3 &p,float radius,int *gridCell){
    for(int i=0;i<8;i++)gridCell[i]=-1;
    // 计算当前粒子光滑核所在网格范围
    int new_min_x=((-radius+p.x-grid_min.x)*grid_offset.x);
    if(new_min_x<0)new_min_x=0;
    int new_min_y=((-radius+p.y-grid_min.y)*grid_offset.y);
    if(new_min_y<0)new_min_y=0;
    int new_min_z=((-radius+p.z-grid_min.z)*grid_offset.z);
    if(new_min_z<0)new_min_z=0;
    // 获取邻域2*2*2个网格
    gridCell[0]=(new_min_z*grid_standards.y+new_min_y)*grid_standards.x+new_min_x;
    gridCell[1]=gridCell[0]+1;
    gridCell[2]=gridCell[0]+grid_standards.x;
    gridCell[3]=gridCell[2]+1;
    if(new_min_z+1<grid_standards.z){
        gridCell[4]=gridCell[0]+grid_standards.y*grid_standards.x;
        gridCell[5]=gridCell[4]+1;
        gridCell[6]=gridCell[4]+grid_standards.x;
        gridCell[7]=gridCell[6]+1;
    }
    if(new_min_x+1>=grid_standards.x){
        gridCell[1]=-1;
        gridCell[3]=-1;
        gridCell[5]=-1;
        gridCell[7]=-1;
    }
    if(new_min_y>=grid_standards.y){
        gridCell[2]=-1;
        gridCell[3]=-1;
        gridCell[6]=-1;
        gridCell[7]=-1;
    }
}
