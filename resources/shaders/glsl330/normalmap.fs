#version 330 core

in vec4 vcolor;
in vec2 UV;
in vec3 Position_worldspace;
in vec3 LightDirection_tangentspace;

out vec4 color;

uniform sampler2D texture0;
uniform sampler2D texture2;
uniform vec3 LightPosition_worldspace;
uniform vec3 LightColor;
uniform float LightPower;

void main()
{
    vec3 diffuseColor = texture(texture0, UV).rgb;
    vec3 ambientColor = vec3(0.1)*diffuseColor;
    vec3 normal = normalize(texture(texture2, UV).rgb*2.0 - 1.0);
    vec3 lightDirection = normalize(LightDirection_tangentspace);
    float diffuse = clamp(dot(normal, lightDirection), 0.0, 1.0);
    float distanceToLight = length(LightPosition_worldspace - Position_worldspace);
    vec3 directColor = vcolor.rgb*diffuseColor*LightColor*LightPower*diffuse/
        max(distanceToLight*distanceToLight, 0.001);
    color = vec4(ambientColor + directColor, vcolor.a);
}
