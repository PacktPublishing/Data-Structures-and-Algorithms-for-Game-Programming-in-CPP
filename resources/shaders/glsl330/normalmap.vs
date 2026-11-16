#version 330 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec2 vertexTexCoord;
layout(location = 2) in vec3 vertexNormal;
layout(location = 4) in vec4 vertexTangent;

uniform mat4 matView;
uniform mat4 matModel;
uniform mat4 matProjection;
uniform vec4 colDiffuse;
uniform vec3 LightPosition_worldspace;

out vec4 vcolor;
out vec2 UV;
out vec3 Position_worldspace;
out vec3 LightDirection_tangentspace;

void main()
{
    Position_worldspace = (matModel*vec4(vertexPosition, 1.0)).xyz;
    vec3 cameraPosition = (matView*vec4(Position_worldspace, 1.0)).xyz;
    gl_Position = matProjection*vec4(cameraPosition, 1.0);
    UV = vertexTexCoord;

    mat3 modelView3 = mat3(matView*matModel);
    mat3 normalMatrix = transpose(inverse(modelView3));
    vec3 normal = normalize(normalMatrix*vertexNormal);
    vec3 tangent = normalize(modelView3*vertexTangent.xyz);
    tangent = normalize(tangent - normal*dot(normal, tangent));
    vec3 bitangent = normalize(cross(normal, tangent))*vertexTangent.w;

    vec3 lightCamera = (matView*vec4(LightPosition_worldspace, 1.0)).xyz - cameraPosition;
    LightDirection_tangentspace = vec3(
        dot(lightCamera, tangent),
        dot(lightCamera, bitangent),
        dot(lightCamera, normal));

    vcolor = colDiffuse;
}
