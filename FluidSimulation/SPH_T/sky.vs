//天空盒的渲染需要使用它自己的着色器程序
//这个着色器需要传入几个属性：一个用于对天空盒进行变换的 WVP 矩阵和其渲染需要用到的纹理单元
//使用 WVP 矩阵对传入的顶点数据进行变换
//需要注意的是我们并不是直接将变换之后的坐标传入 gl_Position 中，在传入前用其 W 分量替换了 Z 分量的值
//因为在光栅化阶段中光栅器会对 gl_Position 执行透视除法（除以其 W 分量），将 Z 分量替换成 W 分量的值，则在透视除法之后这个向量的 Z 分量的值就成为了 1.0
//这样这个片元就会始终位于远裁剪面上，这意味着在深度测试的过程中，天空盒的片元和场景中任意模型的片元比较都会失败。这样天空盒就只会是填满场景模型留下的背景空隙，这也是我们希望得到的效果
#version 330 core
layout (location=0) in vec3 aPos;
layout (location=1) in vec3 aNormal;
out vec3 TexCoords;

uniform mat4 projection;
uniform mat4 view;

void main(){
    TexCoords=aPos;
    vec4 pos=projection*view*vec4(aPos,1.0);
    gl_Position=pos.xyww;
}
