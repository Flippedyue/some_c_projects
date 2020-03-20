#include "MarchingCube.h"

MarchingCube::MarchingCube(){
    m_Grid.fMin=glm::vec3(0.0);
    m_Grid.grid_width=glm::vec3(0.0);
    m_Grid.iNum[0]=0;
    m_Grid.iNum[1]=0;
    m_Grid.iNum[2]=0;
    triangle_cnt=0;
    normal_cnt=0;
    vertex_cnt=0;
    m_ptScalarField=nullptr;
    m_fpScalarFunc=0;
    m_tIsoLevel=0;
    m_bValidSurface=false;
}
MarchingCube::~MarchingCube(){
    DeleteSurface();
}
void MarchingCube::DeleteSurface(){
    m_Grid.grid_width[0]=0;
    m_Grid.grid_width[1]=0;
    m_Grid.grid_width[2]=0;
    m_Grid.iNum[0]=0;
    m_Grid.iNum[1]=0;
    m_Grid.iNum[2]=0;
    triangle_cnt=0;
    normal_cnt=0;
    vertex_cnt=0;
    m_ptScalarField=NULL;
    m_tIsoLevel=0;
    m_bValidSurface=false;
}
bool MarchingCube::CreateMeshV(float *field,glm::vec3 min_p,double h,int *n,float threshold,vector<glm::vec3> &vrts,vector<glm::vec3> &nrms,vector<MCSurface> &face){
    if(field==nullptr)return false;
    MCScalarField sf;
    for(int i=0; i<3; ++i){
        sf.iNum[i]=n[i];
        sf.grid_width[i]=h;
        sf.fMin[i]=min_p[i];
    }
    vector<int> tris;
    GenerateSurfaceV(sf,field,threshold,vrts,nrms,tris);
    if(IsSurfaceValid()){
        int nm=(int)GetNumTriangles();
        int nn=(int)GetNumNormals();
        for(int i=0; i<nn; ++i){
            nrms[i] *= -1.0;
        }      
        face.resize(nm);
        for(int i=0; i<nm; ++i){
            face[i].vert_idx.resize(3);
            for(int j=0; j<3; ++j){
                face[i][j]=tris[3*i+(2-j)];
            }
        }
        new_Nrms.resize(3*face.size(),glm::vec3(0.0));
        for(int i=0;i<face.size();i++){
            unsigned int id0,id1,id2;
            id0=tris[3*i+0];
            id1=tris[3*i+1];
            id2=tris[3*i+2];
            new_Nrms[3*i+0]=nrms[id0];
            new_Nrms[3*i+1]=nrms[id1];
            new_Nrms[3*i+2]=nrms[id2];
        }
        return true;
    }
    return false;
}
void MarchingCube::GenerateSurfaceV(const MCScalarField sf,float *field,float threshold,vector<glm::vec3> &vrts,vector<glm::vec3> &nrms,vector<int> &tris){
    // 等值面生成
    if(m_bValidSurface){
        DeleteSurface();
    }
    m_tIsoLevel=threshold;
    m_Grid.iNum[0]=sf.iNum[0];
    m_Grid.iNum[1]=sf.iNum[1];
    m_Grid.iNum[2]=sf.iNum[2];
    m_Grid.grid_width=sf.grid_width;
    m_Grid.fMin=sf.fMin;
    m_ptScalarField=field;
    unsigned int slice0=(m_Grid.iNum[0]+1);
    unsigned int slice1=slice0*(m_Grid.iNum[1]+1);
    for(unsigned int z=0; z<m_Grid.iNum[2]; ++z){
        for(unsigned int y=0; y<m_Grid.iNum[1]; ++y){
            for(unsigned int x=0; x<m_Grid.iNum[0]; ++x){
                // 计算网格中的顶点放置信息表参考索引
                unsigned int tableIndex=0;
                if(m_ptScalarField[z*slice1+y*slice0+x]<m_tIsoLevel)tableIndex|=1;
                if(m_ptScalarField[z*slice1+(y+1)*slice0+x]<m_tIsoLevel)tableIndex|=2;
                if(m_ptScalarField[z*slice1+(y+1)*slice0+(x+1)]<m_tIsoLevel)tableIndex|=4;
                if(m_ptScalarField[z*slice1+y*slice0+(x+1)]<m_tIsoLevel)tableIndex|=8;
                if(m_ptScalarField[(z+1)*slice1+y*slice0+x]<m_tIsoLevel)tableIndex|=16;
                if(m_ptScalarField[(z+1)*slice1+(y+1)*slice0+x]<m_tIsoLevel)tableIndex|=32;
                if(m_ptScalarField[(z+1)*slice1+(y+1)*slice0+(x+1)]<m_tIsoLevel)tableIndex|=64;
                if(m_ptScalarField[(z+1)*slice1+y*slice0+(x+1)]<m_tIsoLevel)tableIndex|=128;  
                if(edgeTable[tableIndex]!=0){
                    // 计算边上的顶点
                    if(edgeTable[tableIndex]&8){
                        MCVertex pt=CalculateIntersection(x,y,z,3);
                        unsigned int id=GetEdgeID(x,y,z,3);
                        m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                    }
                    if(edgeTable[tableIndex]&1){
                        MCVertex pt=CalculateIntersection(x,y,z,0);
                        unsigned int id=GetEdgeID(x,y,z,0);
                        m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                    }
                    if(edgeTable[tableIndex]&256){
                        MCVertex pt=CalculateIntersection(x,y,z,8);
                        unsigned int id=GetEdgeID(x,y,z,8);
                        m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                    }

                    if(x==m_Grid.iNum[0]-1){
                        if(edgeTable[tableIndex]&4){
                            MCVertex pt=CalculateIntersection(x,y,z,2);
                            unsigned int id=GetEdgeID(x,y,z,2);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                        if(edgeTable[tableIndex]&2048){
                            MCVertex pt=CalculateIntersection(x,y,z,11);
                            unsigned int id=GetEdgeID(x,y,z,11);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    }
                    if(y==m_Grid.iNum[1]-1){
                        if(edgeTable[tableIndex]&2){
                            MCVertex pt=CalculateIntersection(x,y,z,1);
                            unsigned int id=GetEdgeID(x,y,z,1);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                        if(edgeTable[tableIndex]&512){
                            MCVertex pt=CalculateIntersection(x,y,z,9);
                            unsigned int id=GetEdgeID(x,y,z,9);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    }
                    if(z==m_Grid.iNum[2]-1){
                        if(edgeTable[tableIndex]&16){
                            MCVertex pt=CalculateIntersection(x,y,z,4);
                            unsigned int id=GetEdgeID(x,y,z,4);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                        if(edgeTable[tableIndex]&128){
                            MCVertex pt=CalculateIntersection(x,y,z,7);
                            unsigned int id=GetEdgeID(x,y,z,7);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    }
                    if((x==m_Grid.iNum[0]-1)&&(y==m_Grid.iNum[1]-1))
                        if(edgeTable[tableIndex] & 1024){
                            MCVertex pt=CalculateIntersection(x,y,z,10);
                            unsigned int id=GetEdgeID(x,y,z,10);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    if((x==m_Grid.iNum[0]-1)&&(z==m_Grid.iNum[2]-1))
                        if(edgeTable[tableIndex] & 64){
                            MCVertex pt=CalculateIntersection(x,y,z,6);
                            unsigned int id=GetEdgeID(x,y,z,6);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    if((y==m_Grid.iNum[1]-1)&&(z==m_Grid.iNum[2]-1))
                        if(edgeTable[tableIndex] & 32){
                            MCVertex pt=CalculateIntersection(x,y,z,5);
                            unsigned int id=GetEdgeID(x,y,z,5);
                            m_i2pt3idVertices.insert(EdgeToVertex::value_type(id,pt));
                        }
                    // 多边形生成
                    for(unsigned int i=0;triTable[tableIndex][i]!=255;i+=3){
                        MCTriangle triangle;
                        unsigned int triangle_id0,triangle_id1,triangle_id2;
                        triangle_id0=GetEdgeID(x,y,z,triTable[tableIndex][i]);
                        triangle_id1=GetEdgeID(x,y,z,triTable[tableIndex][i+1]);
                        triangle_id2=GetEdgeID(x,y,z,triTable[tableIndex][i+2]);
                        triangle.triangle_id[0]=triangle_id0;
                        triangle.triangle_id[1]=triangle_id1;
                        triangle.triangle_id[2]=triangle_id2;
                        m_trivecTriangles.push_back(triangle);
                    }
                }
            }
        }
    }
    RenameVerticesAndTriangles(vrts,vertex_cnt,tris,triangle_cnt);
    CalculateNormals(vrts,vertex_cnt,tris,triangle_cnt,nrms,normal_cnt);

    m_bValidSurface=true;
}
unsigned int MarchingCube::GetEdgeID(unsigned int nX,unsigned int nY,unsigned int nZ,unsigned int nEdgeNo){
    switch(nEdgeNo){
        case 0:return GetVertexID(nX,nY,nZ)+1;
        case 1:return GetVertexID(nX,nY+1,nZ);
        case 2:return GetVertexID(nX+1,nY,nZ)+1;
        case 3:return GetVertexID(nX,nY,nZ);
        case 4:return GetVertexID(nX,nY,nZ+1)+1;
        case 5:return GetVertexID(nX,nY+1,nZ+1);
        case 6:return GetVertexID(nX+1,nY,nZ+1)+1;
        case 7:return GetVertexID(nX,nY,nZ+1);
        case 8:return GetVertexID(nX,nY,nZ)+2;
        case 9:return GetVertexID(nX,nY+1,nZ)+2;
        case 10:return GetVertexID(nX+1,nY+1,nZ)+2;
        case 11:return GetVertexID(nX+1,nY,nZ)+2;
        default:return -1;
    }
}
unsigned int MarchingCube::GetVertexID(unsigned int nX,unsigned int nY,unsigned int nZ){
    return 3*(nZ*(m_Grid.iNum[1]+1)*(m_Grid.iNum[0]+1)+nY*(m_Grid.iNum[0]+1)+nX);
}
MCVertex MarchingCube::CalculateIntersection(unsigned int nX,unsigned int nY,unsigned int nZ,unsigned int nEdgeNo){
    double x1,y1,z1,x2,y2,z2;
    unsigned int v1x=nX,v1y=nY,v1z=nZ;
    unsigned int v2x=nX,v2y=nY,v2z=nZ;
    switch(nEdgeNo){
        case 0:
            v2y+=1;
            break;
        case 1:
            v1y+=1;
            v2x+=1;
            v2y+=1;
            break;
        case 2:
            v1x+=1;
            v1y+=1;
            v2x+=1;
            break;
        case 3:
            v1x+=1;
            break;
        case 4:
            v1z+=1;
            v2y+=1;
            v2z+=1;
            break;
        case 5:
            v1y+=1;
            v1z+=1;
            v2x+=1;
            v2y+=1;
            v2z+=1;
            break;
        case 6:
            v1x+=1;
            v1y+=1;
            v1z+=1;
            v2x+=1;
            v2z+=1;
            break;
        case 7:
            v1x+=1;
            v1z+=1;
            v2z+=1;
            break;
        case 8:
            v2z+=1;
            break;
        case 9:
            v1y+=1;
            v2y+=1;
            v2z+=1;
            break;
        case 10:
            v1x+=1;
            v1y+=1;
            v2x+=1;
            v2y+=1;
            v2z+=1;
            break;
        case 11:
            v1x+=1;
            v2x+=1;
            v2z+=1;
            break;
    }
    // 获取边的两点坐标
    x1=m_Grid.fMin[0]+v1x*m_Grid.grid_width[0];
    y1=m_Grid.fMin[1]+v1y*m_Grid.grid_width[1];
    z1=m_Grid.fMin[2]+v1z*m_Grid.grid_width[2];
    x2=m_Grid.fMin[0]+v2x*m_Grid.grid_width[0];
    y2=m_Grid.fMin[1]+v2y*m_Grid.grid_width[1];
    z2=m_Grid.fMin[2]+v2z*m_Grid.grid_width[2];
    unsigned int slice0=(m_Grid.iNum[0]+1);
    unsigned int slice1=slice0*(m_Grid.iNum[1]+1);
    float val1=m_ptScalarField[v1z*slice1+v1y*slice0+v1x];
    float val2=m_ptScalarField[v2z*slice1+v2y*slice0+v2x];
    MCVertex intersection=Interpolate(x1,y1,z1,x2,y2,z2,val1,val2);
    return intersection;
}
MCVertex MarchingCube::Interpolate(double fX1,double fY1,double fZ1,double fX2,double fY2,double fZ2,float tVal1,float tVal2){
    MCVertex interpolation;
    float mu;
    mu=float((m_tIsoLevel-tVal1))/(tVal2-tVal1);
    interpolation.x=fX1+mu*(fX2-fX1);
    interpolation.y=fY1+mu*(fY2-fY1);
    interpolation.z=fZ1+mu*(fZ2-fZ1);
    return interpolation;
}
void MarchingCube::RenameVerticesAndTriangles(vector<glm::vec3> &vrts,unsigned int &nvrts,vector<int> &tris,unsigned int &ntris){
    unsigned int nextID=0;
    EdgeToVertex::iterator mapIterator=m_i2pt3idVertices.begin();
    TriangleVector::iterator vecIterator=m_trivecTriangles.begin();
    // 刷新点
    while(mapIterator!=m_i2pt3idVertices.end()){
        (*mapIterator).second.vertex_id=nextID;
        nextID++;
        mapIterator++;
    }
    // 刷新三角面.
    while(vecIterator!=m_trivecTriangles.end()){
        for(unsigned int i=0; i<3; i++){
            unsigned int vertex_id=m_i2pt3idVertices[(*vecIterator).triangle_id[i]].vertex_id;
            (*vecIterator).triangle_id[i]=vertex_id;
        }
        vecIterator++;
    }
    // 将所有顶点和三角形复制到两个数组中，以便可以有效地访问它们。
    // 复制点
    mapIterator=m_i2pt3idVertices.begin();
    nvrts=(int)m_i2pt3idVertices.size();
    vrts.resize(nvrts);
    for(unsigned int i=0; i<nvrts; i++,mapIterator++){
        vrts[i][0]=(*mapIterator).second.x;
        vrts[i][1]=(*mapIterator).second.y;
        vrts[i][2]=(*mapIterator).second.z;
    }
    // 复制制作三角形的顶点索引。
    vecIterator=m_trivecTriangles.begin();
    ntris=(int)m_trivecTriangles.size();
    tris.resize(ntris*3);
    for(unsigned int i=0; i<ntris; i++,vecIterator++){
        tris[3*i+0]=(*vecIterator).triangle_id[0];
        tris[3*i+1]=(*vecIterator).triangle_id[1];
        tris[3*i+2]=(*vecIterator).triangle_id[2];
    }
    //释放空间
    m_i2pt3idVertices.clear();
    m_trivecTriangles.clear();
}
void MarchingCube::CalculateNormals(const vector<glm::vec3> &vrts,unsigned int nvrts,const vector<int> &tris,unsigned int ntris,vector<glm::vec3> &nrms,unsigned int &nnrms){
    nnrms=nvrts;
    nrms.resize(nnrms,glm::vec3(0.0));
    // 计算法线
    for(unsigned int i=0;i<ntris;i++){
        glm::vec3 vec1(0.0),vec2(0.0),normal(0.0);
        unsigned int id0,id1,id2;
        id0=tris[3*i+0];
        id1=tris[3*i+1];
        id2=tris[3*i+2];
        vec1=vrts[id1]-vrts[id0];
        vec2=vrts[id2]-vrts[id0];
        normal=glm::cross(vec1,vec2);
        nrms[id0]+=glm::normalize(normal);
        nrms[id1]+=glm::normalize(normal);
        nrms[id2]+=glm::normalize(normal);
    }
     //单位化法线.
    for(int i=0;i<nnrms;i++)nrms[i]=glm::normalize(nrms[i]);
}
