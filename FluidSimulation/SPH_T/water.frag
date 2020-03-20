#version 330 core
out vec4 FragColor;

//在片段着色器中定义相应的输入变量
in vec3 Normal;
in vec3 Position;
//为了得到观察者的世界空间坐标，我们简单地使用摄像机对象的位置坐标代替
uniform vec3 cameraPos;
uniform samplerCube skybox;

void main(){
    float ratio=1.00/1.33;
    // 计算视线方向向量，和对应的沿着法线轴的反射向量
    vec3 I=normalize(Position-cameraPos);
    vec3 R1=reflect(I,normalize(Normal));
    vec3 R2=refract(I,normalize(Normal),ratio);
    // vec4 allColor=vec4(1.0,1.0,1.0,1.0);
    vec4 allColor=vec4(texture(skybox,R1).rgb*0.4+texture(skybox,R2).rgb*0.6,1.0);
    FragColor=allColor;
}
