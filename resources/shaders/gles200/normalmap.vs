#version 100
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;
attribute vec3 vertexTangent;
uniform mat4 matView;
uniform mat4 matModel;
uniform mat4 matProjection;
uniform vec4 colDiffuse;
uniform vec3 LightPosition_worldspace;
varying vec4 vcolor;
varying vec2 UV;
varying vec3 Position_worldspace;
varying vec3 LightDirection_tangentspace;
varying vec3 EyeDirection_tangentspace;
void main() {
    Position_worldspace = (matModel*vec4(vertexPosition, 1.0)).xyz;
    vec3 cameraPosition = (matView*vec4(Position_worldspace, 1.0)).xyz;
    gl_Position = matProjection*vec4(cameraPosition, 1.0);
    UV = vertexTexCoord;
    mat3 modelView3 = mat3(matView*matModel);
    vec3 tangent = normalize(modelView3*vertexTangent);
    vec3 normal = normalize(modelView3*vertexNormal);
    vec3 bitangent = normalize(cross(normal, tangent));
    vec3 lightCamera = (matView*vec4(LightPosition_worldspace, 1.0)).xyz - cameraPosition;
    vec3 eyeCamera = -cameraPosition;
    LightDirection_tangentspace = vec3(dot(lightCamera, tangent), dot(lightCamera, bitangent), dot(lightCamera, normal));
    EyeDirection_tangentspace = vec3(dot(eyeCamera, tangent), dot(eyeCamera, bitangent), dot(eyeCamera, normal));
    vcolor = colDiffuse;
}
