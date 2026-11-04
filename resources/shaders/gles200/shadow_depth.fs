#version 100
precision mediump float;
varying vec2 fragTexCoord;
uniform sampler2D texture0;
uniform int alphaTest;
void main() {
    vec4 texel = texture2D(texture0, fragTexCoord);
    if (alphaTest > 0 && texel.a < 0.5) discard;
    gl_FragColor = vec4(vec3(gl_FragCoord.z), 1.0);
}
