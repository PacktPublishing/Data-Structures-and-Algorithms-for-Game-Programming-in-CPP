#version 100
precision mediump float;
varying vec3 fragPosition;
varying vec3 fragNormal;
varying vec2 fragTexCoord;
uniform sampler2D texture0;
// WebGL 1 has no core multiple-render-target G-buffer; emit albedo as a fallback.
void main() { gl_FragColor = texture2D(texture0, fragTexCoord); }
