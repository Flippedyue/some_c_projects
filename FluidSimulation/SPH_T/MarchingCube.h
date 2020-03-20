#ifndef MarchingCube_h
#define MarchingCube_h

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <stdio.h>
#include <map>
#include <string>
#include <vector>
#include <iterator>
#include <unordered_map>
using namespace std;

// 密度场是标量场，因此利用密度场计算流体表面

// 由于我们绘制三角形的时候，是先根据边表，判断点落在哪条边上，然后通过插值计算出具体位置，
// 然后我们可以通过EdgeToVertex类对象保存以当前边为索引，索引的值为当前点点具体位置。
// 而由于我们的三角形表获取的信息是点在哪条边上，所以我们用TriangleVector类对象从三角形表获取的边信息，
// 加上通过EdgeToVertex类对象从边信息获取的点具体位置，便可以获得最终三角形片面的三个顶点的最终位置了。

// 面
class MCSurface{
public:
    vector<int> vert_idx;//顶点索引
    vector<glm::vec2> texcoords;//像素坐标
    MCSurface(){}
    inline int& operator[](int i){return vert_idx[i];}
    inline int  operator[](int i)const{return vert_idx[i];}
    inline int& at(int i){return vert_idx.at(i);}//访问顶点
    inline int  at(int i)const{return vert_idx.at(i);}
    void resize(int size){vert_idx.resize(size);}//更改顶点数量
    int size(void) const{return (int)vert_idx.size();}//返回顶点数量
    // 初始化
    void clear(void){
        vert_idx.clear();
        texcoords.clear();
    }
};
// 点
struct MCVertex{
    unsigned int vertex_id;//id
    double x,y,z;//位置
};
// 网格边映射到表面点
typedef std::map<unsigned int,MCVertex> EdgeToVertex;
// 三角片面结构体
struct MCTriangle{
    unsigned int triangle_id[3];//三个顶点索引
};
// 三角面片
typedef std::vector<MCTriangle> TriangleVector;
// 标量场
struct MCScalarField{
    unsigned int iNum[3];//x,y,z的大小
    glm::vec3 grid_width;//每个网格宽度
    glm::vec3 fMin;//最小点坐标
};
struct virToNrm {
    glm::vec3 v;
    glm::vec3 n;
};
class MarchingCube{
private:
    unsigned int vertex_cnt;//点的个数
    unsigned int normal_cnt;//法线个数
    unsigned int triangle_cnt;//三角面个数
    EdgeToVertex m_i2pt3idVertices;//形成等值面的顶点列表(以边索引为key，等值面的点为value)
    TriangleVector m_trivecTriangles;//形成三角形的顶点列表
    MCScalarField m_Grid;//标量场信息
    //用于获取隐式函数值的变量（标量值）
    const float*m_ptScalarField;//保存标量值的样本量
    float (* m_fpScalarFunc)(double,double,double);//返回标量值的函数指针
    float m_tIsoLevel;//阈值
    bool m_bValidSurface;//表面是否生成成功
    vector<glm::vec3> new_Nrms;
    vector<virToNrm> nrm_trans;
    vector<int> nrm_list;//存nrm表的索引
    unsigned int GetEdgeID(unsigned int nX,unsigned int nY,unsigned int nZ,unsigned int nEdgeNo);//边id
    unsigned int GetVertexID(unsigned int nX,unsigned int nY,unsigned int nZ);//顶点id
    MCVertex CalculateIntersection(unsigned int nX,unsigned int nY,unsigned int nZ,unsigned int nEdgeNo);//计算边缘上的等值面
    // 通过网格边缘两端的隐式函数值的线性插值计算等值点
    MCVertex Interpolate(double fX1,double fY1,double fZ1,double fX2,double fY2,double fZ2,float tVal1,float tVal2);
    // 以输出形式存储顶点和网格几何信息
    void RenameVerticesAndTriangles(vector<glm::vec3> &vrts,unsigned int &nvrts,vector<int> &tris,unsigned int &ntris);
    // 顶点法线计算
    void CalculateNormals(const vector<glm::vec3> &vrts,unsigned int nvrts,const vector<int> &tris,unsigned int ntris,vector<glm::vec3> &nrms,unsigned int &nnrms);
public:
    MarchingCube();
    ~MarchingCube();
    //从样本量生成三角形网格
    bool CreateMeshV(float *field,glm::vec3 min_p,double h,int n[3],float threshold,vector<glm::vec3> &vrts,vector<glm::vec3> &nrms,vector<MCSurface> &face);   
    //从样本量生成等值面网格
    void GenerateSurfaceV(const MCScalarField sf,float *field,float threshold,vector<glm::vec3> &vrts,vector<glm::vec3> &nrms,vector<int> &tris); 
    //等值面创建成功则返回true
    bool IsSurfaceValid()const{return m_bValidSurface;} 
    void DeleteSurface();//删除表面
    //获取法线
    vector<glm::vec3>* getNewNrms(){return &new_Nrms;}
    //用于网格划分的网格大小（在未创建网格的情况下，返回值为-1）
    int GetVolumeLengths(double& fVolLengthX,double& fVolLengthY,double& fVolLengthZ);
    //有关创建的网格的信息
    unsigned int GetNumVertices(void)const{return vertex_cnt;}
    unsigned int GetNumTriangles(void)const{return triangle_cnt;}
    unsigned int GetNumNormals(void)const{return normal_cnt;}
};

// 边表
const unsigned int edgeTable[256]={
    0x0  ,0x109,0x203,0x30a,0x406,0x50f,0x605,0x70c,
    0x80c,0x905,0xa0f,0xb06,0xc0a,0xd03,0xe09,0xf00,
    0x190,0x99 ,0x393,0x29a,0x596,0x49f,0x795,0x69c,
    0x99c,0x895,0xb9f,0xa96,0xd9a,0xc93,0xf99,0xe90,
    0x230,0x339,0x33 ,0x13a,0x636,0x73f,0x435,0x53c,
    0xa3c,0xb35,0x83f,0x936,0xe3a,0xf33,0xc39,0xd30,
    0x3a0,0x2a9,0x1a3,0xaa ,0x7a6,0x6af,0x5a5,0x4ac,
    0xbac,0xaa5,0x9af,0x8a6,0xfaa,0xea3,0xda9,0xca0,
    0x460,0x569,0x663,0x76a,0x66 ,0x16f,0x265,0x36c,
    0xc6c,0xd65,0xe6f,0xf66,0x86a,0x963,0xa69,0xb60,
    0x5f0,0x4f9,0x7f3,0x6fa,0x1f6,0xff ,0x3f5,0x2fc,
    0xdfc,0xcf5,0xfff,0xef6,0x9fa,0x8f3,0xbf9,0xaf0,
    0x650,0x759,0x453,0x55a,0x256,0x35f,0x55 ,0x15c,
    0xe5c,0xf55,0xc5f,0xd56,0xa5a,0xb53,0x859,0x950,
    0x7c0,0x6c9,0x5c3,0x4ca,0x3c6,0x2cf,0x1c5,0xcc ,
    0xfcc,0xec5,0xdcf,0xcc6,0xbca,0xac3,0x9c9,0x8c0,
    0x8c0,0x9c9,0xac3,0xbca,0xcc6,0xdcf,0xec5,0xfcc,
    0xcc ,0x1c5,0x2cf,0x3c6,0x4ca,0x5c3,0x6c9,0x7c0,
    0x950,0x859,0xb53,0xa5a,0xd56,0xc5f,0xf55,0xe5c,
    0x15c,0x55 ,0x35f,0x256,0x55a,0x453,0x759,0x650,
    0xaf0,0xbf9,0x8f3,0x9fa,0xef6,0xfff,0xcf5,0xdfc,
    0x2fc,0x3f5,0xff ,0x1f6,0x6fa,0x7f3,0x4f9,0x5f0,
    0xb60,0xa69,0x963,0x86a,0xf66,0xe6f,0xd65,0xc6c,
    0x36c,0x265,0x16f,0x66 ,0x76a,0x663,0x569,0x460,
    0xca0,0xda9,0xea3,0xfaa,0x8a6,0x9af,0xaa5,0xbac,
    0x4ac,0x5a5,0x6af,0x7a6,0xaa ,0x1a3,0x2a9,0x3a0,
    0xd30,0xc39,0xf33,0xe3a,0x936,0x83f,0xb35,0xa3c,
    0x53c,0x435,0x73f,0x636,0x13a,0x33 ,0x339,0x230,
    0xe90,0xf99,0xc93,0xd9a,0xa96,0xb9f,0x895,0x99c,
    0x69c,0x795,0x49f,0x596,0x29a,0x393,0x99 ,0x190,
    0xf00,0xe09,0xd03,0xc0a,0xb06,0xa0f,0x905,0x80c,
    0x70c,0x605,0x50f,0x406,0x30a,0x203,0x109,0x0
};


// 三角形表
#define X 255
const int triTable[256][16]={
    {X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,8,3,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,1,9,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {1,8,3,9,8,1,X,X,X,X,X,X,X,X,X,X},
    {1,2,10,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,8,3,1,2,10,X,X,X,X,X,X,X,X,X,X},
    {9,2,10,0,2,9,X,X,X,X,X,X,X,X,X,X},
    {2,8,3,2,10,8,10,9,8,X,X,X,X,X,X,X},
    {3,11,2,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,11,2,8,11,0,X,X,X,X,X,X,X,X,X,X},
    {1,9,0,2,3,11,X,X,X,X,X,X,X,X,X,X},
    {1,11,2,1,9,11,9,8,11,X,X,X,X,X,X,X},
    {3,10,1,11,10,3,X,X,X,X,X,X,X,X,X,X},
    {0,10,1,0,8,10,8,11,10,X,X,X,X,X,X,X},
    {3,9,0,3,11,9,11,10,9,X,X,X,X,X,X,X},
    {9,8,10,10,8,11,X,X,X,X,X,X,X,X,X,X},
    {4,7,8,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {4,3,0,7,3,4,X,X,X,X,X,X,X,X,X,X},
    {0,1,9,8,4,7,X,X,X,X,X,X,X,X,X,X},
    {4,1,9,4,7,1,7,3,1,X,X,X,X,X,X,X},
    {1,2,10,8,4,7,X,X,X,X,X,X,X,X,X,X},
    {3,4,7,3,0,4,1,2,10,X,X,X,X,X,X,X},
    {9,2,10,9,0,2,8,4,7,X,X,X,X,X,X,X},
    {2,10,9,2,9,7,2,7,3,7,9,4,X,X,X,X},
    {8,4,7,3,11,2,X,X,X,X,X,X,X,X,X,X},
    {11,4,7,11,2,4,2,0,4,X,X,X,X,X,X,X},
    {9,0,1,8,4,7,2,3,11,X,X,X,X,X,X,X},
    {4,7,11,9,4,11,9,11,2,9,2,1,X,X,X,X},
    {3,10,1,3,11,10,7,8,4,X,X,X,X,X,X,X},
    {1,11,10,1,4,11,1,0,4,7,11,4,X,X,X,X},
    {4,7,8,9,0,11,9,11,10,11,0,3,X,X,X,X},
    {4,7,11,4,11,9,9,11,10,X,X,X,X,X,X,X},
    {9,5,4,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {9,5,4,0,8,3,X,X,X,X,X,X,X,X,X,X},
    {0,5,4,1,5,0,X,X,X,X,X,X,X,X,X,X},
    {8,5,4,8,3,5,3,1,5,X,X,X,X,X,X,X},
    {1,2,10,9,5,4,X,X,X,X,X,X,X,X,X,X},
    {3,0,8,1,2,10,4,9,5,X,X,X,X,X,X,X},
    {5,2,10,5,4,2,4,0,2,X,X,X,X,X,X,X},
    {2,10,5,3,2,5,3,5,4,3,4,8,X,X,X,X},
    {9,5,4,2,3,11,X,X,X,X,X,X,X,X,X,X},
    {0,11,2,0,8,11,4,9,5,X,X,X,X,X,X,X},
    {0,5,4,0,1,5,2,3,11,X,X,X,X,X,X,X},
    {2,1,5,2,5,8,2,8,11,4,8,5,X,X,X,X},
    {10,3,11,10,1,3,9,5,4,X,X,X,X,X,X,X},
    {4,9,5,0,8,1,8,10,1,8,11,10,X,X,X,X},
    {5,4,0,5,0,11,5,11,10,11,0,3,X,X,X,X},
    {5,4,8,5,8,10,10,8,11,X,X,X,X,X,X,X},
    {9,7,8,5,7,9,X,X,X,X,X,X,X,X,X,X},
    {9,3,0,9,5,3,5,7,3,X,X,X,X,X,X,X},
    {0,7,8,0,1,7,1,5,7,X,X,X,X,X,X,X},
    {1,5,3,3,5,7,X,X,X,X,X,X,X,X,X,X},
    {9,7,8,9,5,7,10,1,2,X,X,X,X,X,X,X},
    {10,1,2,9,5,0,5,3,0,5,7,3,X,X,X,X},
    {8,0,2,8,2,5,8,5,7,10,5,2,X,X,X,X},
    {2,10,5,2,5,3,3,5,7,X,X,X,X,X,X,X},
    {7,9,5,7,8,9,3,11,2,X,X,X,X,X,X,X},
    {9,5,7,9,7,2,9,2,0,2,7,11,X,X,X,X},
    {2,3,11,0,1,8,1,7,8,1,5,7,X,X,X,X},
    {11,2,1,11,1,7,7,1,5,X,X,X,X,X,X,X},
    {9,5,8,8,5,7,10,1,3,10,3,11,X,X,X,X},
    {5,7,0,5,0,9,7,11,0,1,0,10,11,10,0,X},
    {11,10,0,11,0,3,10,5,0,8,0,7,5,7,0,X},
    {11,10,5,7,11,5,X,X,X,X,X,X,X,X,X,X},
    {10,6,5,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,8,3,5,10,6,X,X,X,X,X,X,X,X,X,X},
    {9,0,1,5,10,6,X,X,X,X,X,X,X,X,X,X},
    {1,8,3,1,9,8,5,10,6,X,X,X,X,X,X,X},
    {1,6,5,2,6,1,X,X,X,X,X,X,X,X,X,X},
    {1,6,5,1,2,6,3,0,8,X,X,X,X,X,X,X},
    {9,6,5,9,0,6,0,2,6,X,X,X,X,X,X,X},
    {5,9,8,5,8,2,5,2,6,3,2,8,X,X,X,X},
    {2,3,11,10,6,5,X,X,X,X,X,X,X,X,X,X},
    {11,0,8,11,2,0,10,6,5,X,X,X,X,X,X,X},
    {0,1,9,2,3,11,5,10,6,X,X,X,X,X,X,X},
    {5,10,6,1,9,2,9,11,2,9,8,11,X,X,X,X},
    {6,3,11,6,5,3,5,1,3,X,X,X,X,X,X,X},
    {0,8,11,0,11,5,0,5,1,5,11,6,X,X,X,X},
    {3,11,6,0,3,6,0,6,5,0,5,9,X,X,X,X},
    {6,5,9,6,9,11,11,9,8,X,X,X,X,X,X,X},
    {5,10,6,4,7,8,X,X,X,X,X,X,X,X,X,X},
    {4,3,0,4,7,3,6,5,10,X,X,X,X,X,X,X},
    {1,9,0,5,10,6,8,4,7,X,X,X,X,X,X,X},
    {10,6,5,1,9,7,1,7,3,7,9,4,X,X,X,X},
    {6,1,2,6,5,1,4,7,8,X,X,X,X,X,X,X},
    {1,2,5,5,2,6,3,0,4,3,4,7,X,X,X,X},
    {8,4,7,9,0,5,0,6,5,0,2,6,X,X,X,X},
    {7,3,9,7,9,4,3,2,9,5,9,6,2,6,9,X},
    {3,11,2,7,8,4,10,6,5,X,X,X,X,X,X,X},
    {5,10,6,4,7,2,4,2,0,2,7,11,X,X,X,X},
    {0,1,9,4,7,8,2,3,11,5,10,6,X,X,X,X},
    {9,2,1,9,11,2,9,4,11,7,11,4,5,10,6,X},
    {8,4,7,3,11,5,3,5,1,5,11,6,X,X,X,X},
    {5,1,11,5,11,6,1,0,11,7,11,4,0,4,11,X},
    {0,5,9,0,6,5,0,3,6,11,6,3,8,4,7,X},
    {6,5,9,6,9,11,4,7,9,7,11,9,X,X,X,X},
    {10,4,9,6,4,10,X,X,X,X,X,X,X,X,X,X},
    {4,10,6,4,9,10,0,8,3,X,X,X,X,X,X,X},
    {10,0,1,10,6,0,6,4,0,X,X,X,X,X,X,X},
    {8,3,1,8,1,6,8,6,4,6,1,10,X,X,X,X},
    {1,4,9,1,2,4,2,6,4,X,X,X,X,X,X,X},
    {3,0,8,1,2,9,2,4,9,2,6,4,X,X,X,X},
    {0,2,4,4,2,6,X,X,X,X,X,X,X,X,X,X},
    {8,3,2,8,2,4,4,2,6,X,X,X,X,X,X,X},
    {10,4,9,10,6,4,11,2,3,X,X,X,X,X,X,X},
    {0,8,2,2,8,11,4,9,10,4,10,6,X,X,X,X},
    {3,11,2,0,1,6,0,6,4,6,1,10,X,X,X,X},
    {6,4,1,6,1,10,4,8,1,2,1,11,8,11,1,X},
    {9,6,4,9,3,6,9,1,3,11,6,3,X,X,X,X},
    {8,11,1,8,1,0,11,6,1,9,1,4,6,4,1,X},
    {3,11,6,3,6,0,0,6,4,X,X,X,X,X,X,X},
    {6,4,8,11,6,8,X,X,X,X,X,X,X,X,X,X},
    {7,10,6,7,8,10,8,9,10,X,X,X,X,X,X,X},
    {0,7,3,0,10,7,0,9,10,6,7,10,X,X,X,X},
    {10,6,7,1,10,7,1,7,8,1,8,0,X,X,X,X},
    {10,6,7,10,7,1,1,7,3,X,X,X,X,X,X,X},
    {1,2,6,1,6,8,1,8,9,8,6,7,X,X,X,X},
    {2,6,9,2,9,1,6,7,9,0,9,3,7,3,9,X},
    {7,8,0,7,0,6,6,0,2,X,X,X,X,X,X,X},
    {7,3,2,6,7,2,X,X,X,X,X,X,X,X,X,X},
    {2,3,11,10,6,8,10,8,9,8,6,7,X,X,X,X},
    {2,0,7,2,7,11,0,9,7,6,7,10,9,10,7,X},
    {1,8,0,1,7,8,1,10,7,6,7,10,2,3,11,X},
    {11,2,1,11,1,7,10,6,1,6,7,1,X,X,X,X},
    {8,9,6,8,6,7,9,1,6,11,6,3,1,3,6,X},
    {0,9,1,11,6,7,X,X,X,X,X,X,X,X,X,X},
    {7,8,0,7,0,6,3,11,0,11,6,0,X,X,X,X},
    {7,11,6,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {7,6,11,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {3,0,8,11,7,6,X,X,X,X,X,X,X,X,X,X},
    {0,1,9,11,7,6,X,X,X,X,X,X,X,X,X,X},
    {8,1,9,8,3,1,11,7,6,X,X,X,X,X,X,X},
    {10,1,2,6,11,7,X,X,X,X,X,X,X,X,X,X},
    {1,2,10,3,0,8,6,11,7,X,X,X,X,X,X,X},
    {2,9,0,2,10,9,6,11,7,X,X,X,X,X,X,X},
    {6,11,7,2,10,3,10,8,3,10,9,8,X,X,X,X},
    {7,2,3,6,2,7,X,X,X,X,X,X,X,X,X,X},
    {7,0,8,7,6,0,6,2,0,X,X,X,X,X,X,X},
    {2,7,6,2,3,7,0,1,9,X,X,X,X,X,X,X},
    {1,6,2,1,8,6,1,9,8,8,7,6,X,X,X,X},
    {10,7,6,10,1,7,1,3,7,X,X,X,X,X,X,X},
    {10,7,6,1,7,10,1,8,7,1,0,8,X,X,X,X},
    {0,3,7,0,7,10,0,10,9,6,10,7,X,X,X,X},
    {7,6,10,7,10,8,8,10,9,X,X,X,X,X,X,X},
    {6,8,4,11,8,6,X,X,X,X,X,X,X,X,X,X},
    {3,6,11,3,0,6,0,4,6,X,X,X,X,X,X,X},
    {8,6,11,8,4,6,9,0,1,X,X,X,X,X,X,X},
    {9,4,6,9,6,3,9,3,1,11,3,6,X,X,X,X},
    {6,8,4,6,11,8,2,10,1,X,X,X,X,X,X,X},
    {1,2,10,3,0,11,0,6,11,0,4,6,X,X,X,X},
    {4,11,8,4,6,11,0,2,9,2,10,9,X,X,X,X},
    {10,9,3,10,3,2,9,4,3,11,3,6,4,6,3,X},
    {8,2,3,8,4,2,4,6,2,X,X,X,X,X,X,X},
    {0,4,2,4,6,2,X,X,X,X,X,X,X,X,X,X},
    {1,9,0,2,3,4,2,4,6,4,3,8,X,X,X,X},
    {1,9,4,1,4,2,2,4,6,X,X,X,X,X,X,X},
    {8,1,3,8,6,1,8,4,6,6,10,1,X,X,X,X},
    {10,1,0,10,0,6,6,0,4,X,X,X,X,X,X,X},
    {4,6,3,4,3,8,6,10,3,0,3,9,10,9,3,X},
    {10,9,4,6,10,4,X,X,X,X,X,X,X,X,X,X},
    {4,9,5,7,6,11,X,X,X,X,X,X,X,X,X,X},
    {0,8,3,4,9,5,11,7,6,X,X,X,X,X,X,X},
    {5,0,1,5,4,0,7,6,11,X,X,X,X,X,X,X},
    {11,7,6,8,3,4,3,5,4,3,1,5,X,X,X,X},
    {9,5,4,10,1,2,7,6,11,X,X,X,X,X,X,X},
    {6,11,7,1,2,10,0,8,3,4,9,5,X,X,X,X},
    {7,6,11,5,4,10,4,2,10,4,0,2,X,X,X,X},
    {3,4,8,3,5,4,3,2,5,10,5,2,11,7,6,X},
    {7,2,3,7,6,2,5,4,9,X,X,X,X,X,X,X},
    {9,5,4,0,8,6,0,6,2,6,8,7,X,X,X,X},
    {3,6,2,3,7,6,1,5,0,5,4,0,X,X,X,X},
    {6,2,8,6,8,7,2,1,8,4,8,5,1,5,8,X},
    {9,5,4,10,1,6,1,7,6,1,3,7,X,X,X,X},
    {1,6,10,1,7,6,1,0,7,8,7,0,9,5,4,X},
    {4,0,10,4,10,5,0,3,10,6,10,7,3,7,10,X},
    {7,6,10,7,10,8,5,4,10,4,8,10,X,X,X,X},
    {6,9,5,6,11,9,11,8,9,X,X,X,X,X,X,X},
    {3,6,11,0,6,3,0,5,6,0,9,5,X,X,X,X},
    {0,11,8,0,5,11,0,1,5,5,6,11,X,X,X,X},
    {6,11,3,6,3,5,5,3,1,X,X,X,X,X,X,X},
    {1,2,10,9,5,11,9,11,8,11,5,6,X,X,X,X},
    {0,11,3,0,6,11,0,9,6,5,6,9,1,2,10,X},
    {11,8,5,11,5,6,8,0,5,10,5,2,0,2,5,X},
    {6,11,3,6,3,5,2,10,3,10,5,3,X,X,X,X},
    {5,8,9,5,2,8,5,6,2,3,8,2,X,X,X,X},
    {9,5,6,9,6,0,0,6,2,X,X,X,X,X,X,X},
    {1,5,8,1,8,0,5,6,8,3,8,2,6,2,8,X},
    {1,5,6,2,1,6,X,X,X,X,X,X,X,X,X,X},
    {1,3,6,1,6,10,3,8,6,5,6,9,8,9,6,X},
    {10,1,0,10,0,6,9,5,0,5,6,0,X,X,X,X},
    {0,3,8,5,6,10,X,X,X,X,X,X,X,X,X,X},
    {10,5,6,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {11,5,10,7,5,11,X,X,X,X,X,X,X,X,X,X},
    {11,5,10,11,7,5,8,3,0,X,X,X,X,X,X,X},
    {5,11,7,5,10,11,1,9,0,X,X,X,X,X,X,X},
    {10,7,5,10,11,7,9,8,1,8,3,1,X,X,X,X},
    {11,1,2,11,7,1,7,5,1,X,X,X,X,X,X,X},
    {0,8,3,1,2,7,1,7,5,7,2,11,X,X,X,X},
    {9,7,5,9,2,7,9,0,2,2,11,7,X,X,X,X},
    {7,5,2,7,2,11,5,9,2,3,2,8,9,8,2,X},
    {2,5,10,2,3,5,3,7,5,X,X,X,X,X,X,X},
    {8,2,0,8,5,2,8,7,5,10,2,5,X,X,X,X},
    {9,0,1,5,10,3,5,3,7,3,10,2,X,X,X,X},
    {9,8,2,9,2,1,8,7,2,10,2,5,7,5,2,X},
    {1,3,5,3,7,5,X,X,X,X,X,X,X,X,X,X},
    {0,8,7,0,7,1,1,7,5,X,X,X,X,X,X,X},
    {9,0,3,9,3,5,5,3,7,X,X,X,X,X,X,X},
    {9,8,7,5,9,7,X,X,X,X,X,X,X,X,X,X},
    {5,8,4,5,10,8,10,11,8,X,X,X,X,X,X,X},
    {5,0,4,5,11,0,5,10,11,11,3,0,X,X,X,X},
    {0,1,9,8,4,10,8,10,11,10,4,5,X,X,X,X},
    {10,11,4,10,4,5,11,3,4,9,4,1,3,1,4,X},
    {2,5,1,2,8,5,2,11,8,4,5,8,X,X,X,X},
    {0,4,11,0,11,3,4,5,11,2,11,1,5,1,11,X},
    {0,2,5,0,5,9,2,11,5,4,5,8,11,8,5,X},
    {9,4,5,2,11,3,X,X,X,X,X,X,X,X,X,X},
    {2,5,10,3,5,2,3,4,5,3,8,4,X,X,X,X},
    {5,10,2,5,2,4,4,2,0,X,X,X,X,X,X,X},
    {3,10,2,3,5,10,3,8,5,4,5,8,0,1,9,X},
    {5,10,2,5,2,4,1,9,2,9,4,2,X,X,X,X},
    {8,4,5,8,5,3,3,5,1,X,X,X,X,X,X,X},
    {0,4,5,1,0,5,X,X,X,X,X,X,X,X,X,X},
    {8,4,5,8,5,3,9,0,5,0,3,5,X,X,X,X},
    {9,4,5,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {4,11,7,4,9,11,9,10,11,X,X,X,X,X,X,X},
    {0,8,3,4,9,7,9,11,7,9,10,11,X,X,X,X},
    {1,10,11,1,11,4,1,4,0,7,4,11,X,X,X,X},
    {3,1,4,3,4,8,1,10,4,7,4,11,10,11,4,X},
    {4,11,7,9,11,4,9,2,11,9,1,2,X,X,X,X},
    {9,7,4,9,11,7,9,1,11,2,11,1,0,8,3,X},
    {11,7,4,11,4,2,2,4,0,X,X,X,X,X,X,X},
    {11,7,4,11,4,2,8,3,4,3,2,4,X,X,X,X},
    {2,9,10,2,7,9,2,3,7,7,4,9,X,X,X,X},
    {9,10,7,9,7,4,10,2,7,8,7,0,2,0,7,X},
    {3,7,10,3,10,2,7,4,10,1,10,0,4,0,10,X},
    {1,10,2,8,7,4,X,X,X,X,X,X,X,X,X,X},
    {4,9,1,4,1,7,7,1,3,X,X,X,X,X,X,X},
    {4,9,1,4,1,7,0,8,1,8,7,1,X,X,X,X},
    {4,0,3,7,4,3,X,X,X,X,X,X,X,X,X,X},
    {4,8,7,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {9,10,8,10,11,8,X,X,X,X,X,X,X,X,X,X},
    {3,0,9,3,9,11,11,9,10,X,X,X,X,X,X,X},
    {0,1,10,0,10,8,8,10,11,X,X,X,X,X,X,X},
    {3,1,10,11,3,10,X,X,X,X,X,X,X,X,X,X},
    {1,2,11,1,11,9,9,11,8,X,X,X,X,X,X,X},
    {3,0,9,3,9,11,1,2,9,2,11,9,X,X,X,X},
    {0,2,11,8,0,11,X,X,X,X,X,X,X,X,X,X},
    {3,2,11,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {2,3,8,2,8,10,10,8,9,X,X,X,X,X,X,X},
    {9,10,2,0,9,2,X,X,X,X,X,X,X,X,X,X},
    {2,3,8,2,8,10,0,1,8,1,10,8,X,X,X,X},
    {1,10,2,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {1,3,8,9,1,8,X,X,X,X,X,X,X,X,X,X},
    {0,9,1,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {0,3,8,X,X,X,X,X,X,X,X,X,X,X,X,X},
    {X,X,X,X,X,X,X,X,X,X,X,X,X,X,X,X}
};
#undef X

#endif /* MarchingCube_h */
