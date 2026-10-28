#version 100
precision mediump float;
precision mediump int;
varying vec3 worldPos;
varying vec2 texUV;
varying vec4 vtxColor;
varying vec3 vtxNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int alphaTest;
#define MAX_LIGHTS 4
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
struct Light { int enabled; int type; vec3 position; vec3 target; vec4 color; };
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float materialShininess;
void main() {
    vec4 texel = texture2D(texture0, texUV)*vtxColor;
    if (alphaTest == 1 && texel.a < 0.5) discard;
    vec3 normal = normalize(vtxNormal);
    vec3 viewD = normalize(viewPos - worldPos);
    vec3 lightSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);
    for (int i = 0; i < MAX_LIGHTS; i++) if (lights[i].enabled == 1) {
        vec3 direction = lights[i].type == LIGHT_DIRECTIONAL ? -normalize(lights[i].target - lights[i].position) : normalize(lights[i].position - worldPos);
        float diffuse = max(dot(normal, direction), 0.0);
        lightSum += lights[i].color.rgb*diffuse;
        if (diffuse > 0.0) specularSum += vec3(pow(max(dot(normal, normalize(direction + viewD)), 0.0), materialShininess));
    }
    gl_FragColor = texel*((colDiffuse + vec4(specularSum, 1.0))*vec4(lightSum, 1.0)) + ambient*colDiffuse;
}
