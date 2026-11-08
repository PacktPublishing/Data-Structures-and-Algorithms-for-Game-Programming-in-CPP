#version 100
precision mediump float;
precision mediump int;
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int alphaTest;
#define MAX_LIGHTS 4
#define LIGHT_DIRECTIONAL 0
#define LIGHT_POINT 1
struct Light {
    int enabled; int type; vec3 position; vec3 target; vec4 color;
    float attn_const; float attn_linear; float attn_quad;
};
uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float materialShininess;
uniform vec3 lightDir;
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;
uniform int receiveShadow;
void main() {
    vec4 texel = texture2D(texture0, fragTexCoord);
    if (alphaTest > 0 && texel.a < 0.51) discard;
    vec3 normal = normalize(fragNormal);
    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 lightSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);
    for (int i = 0; i < MAX_LIGHTS; i++) if (lights[i].enabled == 1) {
        vec3 direction = lights[i].type == LIGHT_DIRECTIONAL
            ? -normalize(lights[i].target - lights[i].position)
            : normalize(lights[i].position - fragPosition);
        float diffuse = max(dot(normal, direction), 0.0);
        float attenuation = 1.0;
        if (lights[i].type == LIGHT_POINT) {
            float distanceToLight = length(lights[i].position - fragPosition);
            attenuation = 1.0/(lights[i].attn_const + lights[i].attn_linear*distanceToLight + lights[i].attn_quad*distanceToLight*distanceToLight);
        }
        lightSum += lights[i].color.rgb*diffuse*attenuation;
        if (diffuse > 0.0) specularSum += vec3(pow(max(dot(normal, normalize(direction + viewD)), 0.0), materialShininess));

        if (receiveShadow == 1 && i == 0 && lights[i].type == LIGHT_DIRECTIONAL) {
            vec3 shadowLight = -lightDir;
            vec4 lightSpace = lightVP*vec4(fragPosition, 1.0);
            vec3 projected = lightSpace.xyz/lightSpace.w*0.5 + 0.5;
            // Avoid peter-panning that erases nearby shadows on sloped terrain.
            float bias = max(0.0002*(1.0 - dot(normal, shadowLight)), 0.00002) + 0.00001;
            float shadow = 0.0;
            vec2 texelSize = vec2(1.0/float(shadowMapResolution));

            for (int x = -1; x <= 1; x++) {
                for (int y = -1; y <= 1; y++) {
                    float mapDepth = texture2D(shadowMap,
                        projected.xy + texelSize*vec2(float(x), float(y))).r;
                    if (projected.z - bias > mapDepth) shadow += 1.0;
                }
            }

            lightSum = mix(lightSum, lightSum*ambient.rgb, shadow/9.0);
        }
    }
    gl_FragColor = texel*((colDiffuse + vec4(specularSum, 1.0))*vec4(lightSum, 1.0) + ambient*colDiffuse);
}
