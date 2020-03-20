#ifndef Camera_hpp
#define Camera_hpp

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

// OpenGL本身没有摄像机(Camera)的概念，
// 但我们可以通过把场景中的所有物体往相反方向移动的方式来模拟出摄像机，
// 产生一种我们在移动的感觉，而不是场景在移动。
enum Camera_Movement{
    FORWARD,
    BACKWARD,
    LEFT,
    RIGHT
};
const float YAW=0,PITCH=0,SPEED=2.5,SENSITIVITY=0.1,ZOOM=45;
// 当我们讨论摄像机/观察空间(Camera/View Space)的时候，
// 是在讨论以摄像机的视角作为场景原点时场景中所有的顶点坐标：观察矩阵把所有的世界坐标变换为相对于摄像机位置与方向的观察坐标。
class Camera{
public:
    glm::vec3 Position,Front,Right,WorldUp;
    float Yaw,Pitch,MovementSpeed,MouseSensitivity,Zoom;
    Camera(glm::vec3 position=glm::vec3(0.0,20.0f,50.0f),glm::vec3 up=glm::vec3(0.0f,1.0f,0.0f),float yaw=YAW,float pitch=PITCH){
        Position=position;
        WorldUp=up;
        Yaw=yaw;
        Pitch=pitch;
        Front=glm::vec3(0.0f,0.0f,1.0f);
        MouseSensitivity=SENSITIVITY;
        MovementSpeed=SPEED;
        Zoom=ZOOM;
        updateCameraVectors();
    }
    Camera(float posX,float posY,float posZ,float upX,float upY,float upZ,float yaw,float pitch){
        Position=glm::vec3(posX,posY,posZ);
        WorldUp=glm::vec3(upX,upY,upZ);
        Yaw=yaw;
        Pitch=pitch;
        Front=glm::vec3(0.0f,0.0f,1.0f);
        MouseSensitivity=SENSITIVITY;
        MovementSpeed=SPEED;
        Zoom=ZOOM;
        updateCameraVectors();
    }
    // 通过俯仰角和偏航角来计算以得到真正的方向向量
    void updateCameraVectors(){
        glm::vec3 front;
        front.x=sin(glm::radians(Yaw))*cos(glm::radians(Pitch));//角度转为弧度
        front.y=sin(glm::radians(Pitch));
        front.z=-cos(glm::radians(Yaw))*cos(glm::radians(Pitch));
        Front=glm::normalize(-front);
        Right=glm::normalize(glm::cross(glm::vec3(0.0f,1.0f,0.0f),Front));
        WorldUp=glm::normalize(glm::cross(Front,Right));
    }
    glm::mat4 getViewMatrix(){
        return glm::lookAt(Position,Position-Front,WorldUp);
    }
    // 按键
    void processKeyboard(Camera_Movement direction,float delta_time){
        float velocity=MovementSpeed*delta_time;
        if(direction==FORWARD)Position+=-Front*velocity;
        if(direction==BACKWARD)Position-=-Front*velocity;
        if(direction==LEFT)Position-=Right*velocity;
        if(direction==RIGHT)Position+=Right*velocity;
    }
    void processMouseMovement(float xOffset,float yOffset){
        // 偏移量乘以灵敏度值，这样鼠标移动不会太大
        xOffset *= MouseSensitivity;
        yOffset *= MouseSensitivity;
        Yaw+=xOffset;
        Pitch-=yOffset;
        // 给摄像机添加一些限制
        // 对于俯仰角，要让用户不能看向高于89度的地方（在90度时视角会发生逆转，所以我们把89度作为极限）
        // 同样也不允许小于-89度。这样能够保证用户只能看到天空或脚下，但是不能超越这个限制。
        if(Pitch>89.0f)Pitch=89.0f;
        if(Pitch<-89.0f)Pitch=-89.0f;
        updateCameraVectors();
    }
    //变焦处理，实现一个缩放(Zoom)接口
    void processMouseScroll(float yoffset){
        if(Zoom>=5.0f&&Zoom<=70.0f)Zoom-=yoffset;
        if(Zoom<=5.0f)Zoom=5.0f;
        if(Zoom>=70.0f)Zoom=70.0f;
    }
};
#endif /* Camera_hpp */
