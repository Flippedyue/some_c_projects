//片元着色器
//使用 samplerCube 从 cubemap 进行采样
#version 330 core
out vec4 FragColor;
in vec3 TexCoords;
uniform samplerCube skybox;

void main(){
    FragColor=texture(skybox,TexCoords);
}
