// kn_lit.vs - WebGL 1 / GLSL ES 1.00 lighting vertex shader
#version 100

attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
attribute vec3 vertexNormal;
attribute vec4 vertexColor;

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matNormal;

varying vec3 worldPos;
varying vec2 texUV;
varying vec4 vtxColor;
varying vec3 vtxNormal;

void main()
{
    worldPos = vec3(matModel*vec4(vertexPosition, 1.0));
    texUV = vertexTexCoord;
    vtxColor = vertexColor;
    vtxNormal = normalize(vec3(matNormal*vec4(vertexNormal, 0.0)));
    gl_Position = mvp*vec4(vertexPosition, 1.0);
}
