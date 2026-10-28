#version 100
precision mediump float;
varying vec3 fragPosition;
varying vec3 fragNormal;
uniform vec3 lightPosition;
uniform vec3 lightColor;
uniform vec4 colDiffuse;
void main() {
    vec3 lightDirection = normalize(lightPosition - fragPosition);
    float diffuse = max(dot(normalize(fragNormal), lightDirection), 0.0);
    gl_FragColor = vec4(colDiffuse.rgb*lightColor*diffuse, colDiffuse.a);
}
