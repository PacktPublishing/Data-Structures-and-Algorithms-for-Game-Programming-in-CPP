#version 100
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
attribute vec2 vertexTexCoord;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;
varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
void main() {
    vec4 worldPosition = matModel*vec4(vertexPosition, 1.0);
    fragPosition = worldPosition.xyz;
    fragNormal = normalize(mat3(matModel)*vertexNormal);
    fragTexCoord = vertexTexCoord;
    gl_Position = matProjection*matView*worldPosition;
}
