#ifndef FluidSystem_h
#define FluidSystem_h

#include "Box.h"
#include "NeighborTable.h"
#include "MarchingCube.h"

// 我们用m_mcMesh来控制我们的表面生成，m_field保存密度场，m_thre则为隐函数阈值（阈值内为表面内点，阈值外为表面外点）。
// 还用了m_vrts，m_nrms，m_face来保存表面信息。
// 添加的函数中，主要用函数GetImplicit，CalImplicitField，CalImplicitFieldDevice去计算我们的网格密度值，
// 获取到所有网格的密度值，便可以用mc类获取表面信息。
// 密度场由CalColorField函数计算

void hjh();
bool _less(const int&a,const int& b);
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
    ~FluidSystem();
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
    //===== 表面相关 =====
    // 复制缓存
    void CuCopyArrayToDevice(float* device,const float* host,int offset,int size);
    // 隐式函数值计算
    virtual double GetImplicit(double x,double y,double z);
    virtual void CalImplicitField(int n[3],glm::vec3 minp,glm::vec3 d,float *hF);
    virtual void CalImplicitFieldDevice(int n[3],glm::vec3 minp,glm::vec3 d,float *dF); 
    // 使用metaball隐式函数值
    double CalColorField(double x,double y,double z);
    float* getPointPosBuf(){return &fluid_point_pos[0];}//获取点位置缓存
    float* getPolygonBuf();//获取三角面
    float* getSufPosBuf(){return &m_vrts[0].x;}//获取表面点
    float* getSufPosNrm(){return &m_nrms[0].x;}
    void clearSuf(){m_nrms.clear();m_nrms.clear();m_face.clear();}
    int getSufVrtBufNum(){return 3*m_mcMesh.GetNumVertices();}
    int getSubNrmNum(){return 3*m_mcMesh.GetNumNormals();}
    int getPolyNum();
    float* getAll(){return &m_all[0];}
    int getAllNum(){return m_all.size();}
    void culAll();
private:
    MarchingCube m_mcMesh;
    float* m_field;//密度场
    std::vector<float>m_polyBuf;//三角面缓存
    std::vector<float> m_hPosW;  
    //表面信息
    vector<glm::vec3> m_vrts;//表面点坐标
    vector<glm::vec3> m_nrms;//点法线
    vector<MCSurface> m_face;//表面三角面
    vector<float> m_all;//表面三角面
    float m_thre;//隐函数阈值
};

#endif /* FluidSystem_h */
