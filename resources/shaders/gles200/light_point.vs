#version 100
attribute vec3 vertexPosition;
attribute vec3 vertexNormal;
uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 matProjection;
varying vec3 fragPosition;
varying vec3 fragNormal;
void main() {
    fragPosition = vec3(matModel*vec4(vertexPosition, 1.0));
    fragNormal = normalize(mat3(matModel)*vertexNormal);
    gl_Position = matProjection*matView*vec4(fragPosition, 1.0);
}
