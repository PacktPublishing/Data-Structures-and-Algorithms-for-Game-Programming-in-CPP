#version 100
attribute vec3 vertexPosition;
attribute vec2 vertexTexCoord;
varying vec2 fragTexCoord;
void main() {
    fragTexCoord = vertexTexCoord;
    gl_Position = vec4(vertexPosition, 1.0);
}
