//一个顶点着色器来绘制箱子
#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;

//法向量由顶点着色器传递到片段着色器
out vec3 Normal;
out vec3 Position;

uniform mat4 projection;
uniform mat4 view;
uniform mat4 model;

void main(){
    Normal=mat3(transpose(inverse(model)))*aNormal;
    Position=vec3(model*vec4(aPos,1.0));
    gl_Position=projection*view*model*vec4(aPos,1.0f);
}
