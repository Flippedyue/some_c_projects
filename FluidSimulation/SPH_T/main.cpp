#include "shader.h"
#include "Camera.hpp"
#include "stb_image.h"
#include "FluidSystem.h"
#include <GLFW/glfw3.h>
#include <vector>

#define IDC_TOGGLEFULLSCREEN    1
#define IDC_TOGGLEREF           3
#define IDC_CHANGEDEVICE        4
#define IDC_RESET               5
#define PARTICLE_COUNTS            4096*4

float skyCube[]={
    -1.0f, 1.0f,-1.0f,
    -1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    
    -1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f,-1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    
    -1.0f,-1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f,-1.0f, 1.0f,
    -1.0f,-1.0f, 1.0f,
    
    -1.0f, 1.0f,-1.0f,
    1.0f, 1.0f,-1.0f,
    1.0f, 1.0f, 1.0f,
    1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f, 1.0f,
    -1.0f, 1.0f,-1.0f,
    
    -1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f,-1.0f,
    1.0f,-1.0f,-1.0f,
    -1.0f,-1.0f, 1.0f,
    1.0f,-1.0f, 1.0f
};

Camera my_camera;
float delta_time,last_time;
GLuint winWidth=1440,winHeight=800;
double lastX=winWidth/2,lastY=winWidth/2;
bool FirstMouse=false;
unsigned int sphereVAO=0,skyVAO=0;
unsigned int vbo,skyVBO;
GLuint boxVAO,boxVBO,boxEBO;
unsigned int indexCount;
static FluidSystem staticSystem;
bool B_or_V=1;
FluidSystem *m_system=&staticSystem;

void init();
void framebuffer_size_callback(GLFWwindow* window,int width,int height);
void mouse_pos_callback(GLFWwindow *window,double x,double y);
void mouse_scroll_callback(GLFWwindow *window,double x,double y);
void processInput(GLFWwindow *window);
unsigned int loadCubemap(std::vector<std::string> faces);//加载天空盒
void renderBox();
void renderFluid(Shader s1);
void renderSky();
glm::vec3 wallMax,wallMin;
void restSPHSystem(){
    glm::vec3 wall_min(-20.0,0.0,-20.0);
    glm::vec3 wall_max(20.0,40.0,20.0);
    glm::vec3 fluid_min(-10.0,8,-10.0);
    glm::vec3 fluid_max(15.0,30,15.0);
    glm::vec3 gravity(0.0,-9.8f,0);
    m_system->init(PARTICLE_COUNTS,wall_min,wall_max,fluid_min,fluid_max,gravity);
    wallMax=wall_max;   wallMin=wall_min;
}

int main(){   
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR,3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR,3);
    glfwWindowHint(GLFW_SAMPLES,4);
    glfwWindowHint(GLFW_OPENGL_PROFILE,GLFW_OPENGL_CORE_PROFILE);
    
#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT,GL_TRUE); // uncomment this statement to fix compilation on OS X
#endif
    
    // glfw window creation
    // --------------------
    GLFWwindow* window=glfwCreateWindow(winWidth,winHeight,"LearnOpenGL",NULL,NULL);
    glfwMakeContextCurrent(window);
    if(window==NULL){
        std::cout << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }
    
    glewInit();

    glfwSetFramebufferSizeCallback(window,framebuffer_size_callback);
    // 告诉GLFW，它应该隐藏光标，并捕捉(Capture)它
    glfwSetInputMode(window,GLFW_CURSOR,GLFW_CURSOR_DISABLED);
    // 注册鼠标移动回调函数
    glfwSetCursorPosCallback(window,mouse_pos_callback);
    // 注册鼠标滚轮的回调函数
    glfwSetScrollCallback(window,mouse_scroll_callback);

    init();
    
    // initialize static shader uniforms before rendering
    // --------------------------------------------------
    glm::mat4 projection=glm::mat4(1.0f);
    glm::mat4 view=glm::mat4(1.0f);
    
    Shader shader("/Users/mac/Desktop/FluidSimulation/SPH_T/water.vs","/Users/mac/Desktop/FluidSimulation/SPH_T/water.frag");
    Shader skyShader("/Users/mac/Desktop/FluidSimulation/SPH_T/sky.vs","/Users/mac/Desktop/FluidSimulation/SPH_T/sky.frag");
    
    //加入天空盒
    std::vector<std::string> faces{
        "/Users/mac/Desktop/FluidSimulation/SPH_T/front.tga",
        "/Users/mac/Desktop/FluidSimulation/SPH_T/back.tga",
        "/Users/mac/Desktop/FluidSimulation/SPH_T/top.tga",
        "/Users/mac/Desktop/FluidSimulation/SPH_T/bottom.tga",
        "/Users/mac/Desktop/FluidSimulation/SPH_T/right.tga",
        "/Users/mac/Desktop/FluidSimulation/SPH_T/left.tga"
    };
    unsigned int cubemapTexture=loadCubemap(faces);
    
    // 在使用这个纹理对象来绘制天空盒之前，我们必须调用这个函数，同样我们将这个纹理对象绑定到 GL_TEXTURE_CUBE_MAP 目标上
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_CUBE_MAP,cubemapTexture);
    
    // render loop
    // -----------
    while(!glfwWindowShouldClose(window)){
        // per-frame time logic
        // --------------------
        float currentFrame=glfwGetTime();
        delta_time=currentFrame-last_time;
        //if(delta_time>=0.0){
        // input
        // -----
        processInput(window);
        // render
        // ------
        glClearColor(0.1f,0.1f,0.1f,1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        projection=glm::perspective(glm::radians(my_camera.Zoom),float(winWidth)/float(winHeight),0.1f,200.0f);
        glm::mat4 model=glm::mat4(1.0f);
        view=my_camera.getViewMatrix();

        shader.use();
        shader.setMat4("view",view);
        shader.setMat4("projection",projection);
        shader.setInt("skybox",1);
        shader.setMat4("model",model);
        shader.setVec3("cameraPos",my_camera.Position);
    
        renderBox();
        renderFluid(shader);
        
        view=glm::mat4(glm::mat3(my_camera.getViewMatrix()));
        
        skyShader.use();
        skyShader.setMat4("view",view);
        skyShader.setMat4("projection",projection);
        skyShader.setMat4("model",model);
        skyShader.setInt("skybox",1);
        renderSky();
        skyShader.close();

        glfwSwapBuffers(window);
        glfwPollEvents();
        last_time=currentFrame;
    } 
    // glfw: terminate,clearing all previously allocated GLFW resources.
    // ------------------------------------------------------------------
    glfwTerminate();
    return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void init(){
    glEnable(GL_MULTISAMPLE);//开启多重采样
    glPointSize(5.0);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    restSPHSystem();
}
void framebuffer_size_callback(GLFWwindow* window,int width,int height){
    glViewport(0,0,width,height);
}
// 键盘
void processInput(GLFWwindow *window){
    if(glfwGetKey(window,GLFW_KEY_ESCAPE)==GLFW_PRESS)glfwSetWindowShouldClose(window,true);
    if(glfwGetKey(window,GLFW_KEY_W))my_camera.processKeyboard(FORWARD,delta_time);
    if(glfwGetKey(window,GLFW_KEY_S))my_camera.processKeyboard(BACKWARD,delta_time);
    if(glfwGetKey(window,GLFW_KEY_A))my_camera.processKeyboard(LEFT,delta_time);
    if(glfwGetKey(window,GLFW_KEY_D))my_camera.processKeyboard(RIGHT,delta_time);
    if(glfwGetKey(window,GLFW_KEY_R))restSPHSystem();
    if(glfwGetKey(window,GLFW_KEY_V))B_or_V=0;
    if(glfwGetKey(window,GLFW_KEY_B))B_or_V=1;
}
// 为了计算俯仰角和偏航角，我们需要让GLFW监听鼠标移动事件
// x和y代表当前鼠标的位置。当我们用GLFW注册了回调函数之后，鼠标一移动这个函数就会被调用
void mouse_pos_callback(GLFWwindow *window,double x,double y){
    if(!FirstMouse){
        lastX=x;
        lastY=y;
        FirstMouse=true;
    }
    float xOffset=x-lastX;
    float yOffset=y-lastY;
    lastX=x;
    lastY=y;
    my_camera.processMouseMovement(xOffset,yOffset);
}
void mouse_scroll_callback(GLFWwindow *window,double x,double y){
    my_camera.processMouseScroll(y);
}
void renderSky(){
    glDepthMask(false);
    if(skyVAO==0){
        // 表示灯（光源）的立方体，为这个灯创建一个专门的VAO
        glGenVertexArrays(1,&skyVAO);
        glGenBuffers(1,&skyVBO);
        glBindVertexArray(skyVAO);
        glBindBuffer(GL_ARRAY_BUFFER,skyVBO);
        glBufferData(GL_ARRAY_BUFFER,36*3*sizeof(float),skyCube,GL_DYNAMIC_DRAW);
        float stride=3*sizeof(float);
        // 改一下顶点属性指针来适应新的顶点数组的大小
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
    }
    glBindVertexArray(skyVAO);
    glDrawArrays(GL_TRIANGLES,0,36);
    glBindVertexArray(0);
    glDepthMask(true);
}
void renderFluid(Shader s1){
    float* data=nullptr;
    m_system->tick();
    if(B_or_V){
        data=m_system->getAll();
        if(sphereVAO==0){
        glGenVertexArrays(1,&sphereVAO);
        glGenBuffers(1,&vbo);
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,(m_system->getAllNum())*sizeof(float),&data[0],GL_DYNAMIC_DRAW);
        float stride=3*sizeof(float);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
        glVertexAttribPointer(1,3,GL_FLOAT,GL_FALSE,stride,(void*)(m_system->getPolyNum()*sizeof(float)));
        glEnableVertexAttribArray(0);
        glEnableVertexAttribArray(1);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_TRIANGLES,0,m_system->getPolyNum()/3);
        glBindVertexArray(0);
        s1.use();  
    }
    else{
        data=m_system->getPointPosBuf();//获取所有点
        if(sphereVAO==0){
            glGenVertexArrays(1,&sphereVAO);
            glGenBuffers(1,&vbo);
        }
        glBindVertexArray(sphereVAO);
        glBindBuffer(GL_ARRAY_BUFFER,vbo);
        glBufferData(GL_ARRAY_BUFFER,3*m_system->getPointCnt()*sizeof(float),data,GL_DYNAMIC_DRAW);
        float stride=3*sizeof(float);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
        glEnableVertexAttribArray(0);
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindVertexArray(sphereVAO);
        glDrawArrays(GL_POINTS,0,m_system->getPointCnt());
        glBindVertexArray(0);
    }
}
void renderBox(){
    static bool firstDraw=1;
    glPolygonMode(GL_FRONT_AND_BACK,GL_LINE);
    if(firstDraw){
        glm::vec3 delta=wallMax-wallMin;
        std::vector<glm::vec3>vertex;
        std::vector<float>data;
        vertex.push_back(wallMin);//0
        vertex.push_back(wallMin+glm::vec3(delta.x,0,0));//1
        vertex.push_back(wallMin+glm::vec3(delta.x,delta.y,0));//2
        vertex.push_back(wallMin+glm::vec3(0,delta.y,0));//3
        vertex.push_back(wallMin+glm::vec3(0,0,delta.z));//4
        vertex.push_back(wallMin+glm::vec3(delta.x,0,delta.z));//5
        vertex.push_back(wallMin+glm::vec3(delta.x,delta.y,delta.z));//6
        vertex.push_back(wallMin+glm::vec3(0,delta.y,delta.z));//7
        for(int i=0;i<8;i++){
            data.push_back(vertex[i].x);
            data.push_back(vertex[i].y);
            data.push_back(vertex[i].z);
        }
        GLuint index[30]={0,1,2,3,0,0,3,7,4,0,0,1,5,4,0,1,5,6,2,1,3,2,6,7,3,4,5,6,7,4};
        glGenVertexArrays(1,&boxVAO);
        glGenBuffers(1,&boxVBO);
        glGenBuffers(1,&boxEBO);
        glBindVertexArray(boxVAO);
        glBindBuffer(GL_ARRAY_BUFFER,boxVBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,boxEBO);
        glBufferData(GL_ARRAY_BUFFER,3*m_system->getPointCnt()*sizeof(float),&data[0],GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,30*sizeof(GLuint),index,GL_STATIC_DRAW);
        glEnableVertexAttribArray(0);
        float stride=3*sizeof(float);
        glVertexAttribPointer(0,3,GL_FLOAT,GL_FALSE,stride,(void*)0);
        firstDraw=0;
        glBindBuffer(GL_ARRAY_BUFFER,0);
        glBindVertexArray(0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER,0);
    }
    glBindVertexArray(boxVAO);
    for(int i=0;i<6;i++)glDrawElements(GL_LINE_STRIP,5,GL_UNSIGNED_INT,(void*)(i*5*sizeof(GL_UNSIGNED_INT)));
    glBindVertexArray(0);
    glPolygonMode(GL_FRONT_AND_BACK,GL_FILL);
}
// 导入 cubemap 要用到的纹理
unsigned int loadCubemap(std::vector<std::string> faces){
    unsigned int textureID;
    // 先创建了一个纹理对象，并且这个纹理对象被绑定到了一个特殊的 GL_TEXTURE_CUBE_MAP 目标上
    glGenTextures(1,&textureID);
    glBindTexture(GL_TEXTURE_CUBE_MAP,textureID);
    int width,height,nrChannels;
    // 循环，为 cubemap 的每个面指定其对应的纹理数据
    for (unsigned int i=0; i<faces.size(); i++){
        unsigned char *data=stbi_load(faces[i].c_str(),&width,&height,&nrChannels,0);
        if(data){
            // 通过 glTexImage2D() 函数将数据传递给 OpenGL
            glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X+i,0,GL_RGB,width,height,0,GL_RGB,GL_UNSIGNED_BYTE,data);
            stbi_image_free(data);
        }
        else{
            std::cout << "加载纹理出错: " << faces[i] << std::endl;
            stbi_image_free(data);
        }
    }
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_S,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_T,GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP,GL_TEXTURE_WRAP_R,GL_CLAMP_TO_EDGE);
    return textureID;
}
