#version 100
precision mediump float;
varying mediump vec4 vcolor;
varying mediump vec2 UV;
varying mediump vec3 Position_worldspace;
varying mediump vec3 LightDirection_tangentspace;
varying mediump vec3 EyeDirection_tangentspace;
uniform sampler2D texture0;
uniform sampler2D texture2;
uniform mediump vec3 LightPosition_worldspace;
uniform vec3 LightColor;
uniform float LightPower;
void main() {
    vec3 diffuseColor = texture2D(texture0, UV).rgb;
    vec3 ambientColor = vec3(0.1)*diffuseColor;
    vec3 normal = normalize(texture2D(texture2, UV).rgb*2.0 - 1.0);
    vec3 lightDirection = normalize(LightDirection_tangentspace);
    float diffuse = clamp(dot(normal, lightDirection), 0.0, 1.0);
    float distanceToLight = length(LightPosition_worldspace - Position_worldspace);
    gl_FragColor = vec4(ambientColor, 1.0) + vcolor*vec4(diffuseColor*LightColor*LightPower*diffuse/max(distanceToLight*distanceToLight, 0.001), 1.0);
}
