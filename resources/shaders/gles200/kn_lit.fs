// kn_lit.fs - WebGL 1 / GLSL ES 1.00 lighting fragment shader
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

struct Light {
    int enabled;
    int type;
    vec3 position;
    vec3 target;
    vec4 color;
};

uniform Light lights[MAX_LIGHTS];
uniform vec4 ambient;
uniform vec3 viewPos;
uniform float materialShininess;

void main()
{
    vec4 texel = texture2D(texture0, texUV)*vtxColor;
    if (alphaTest == 1 && texel.a < 0.5) discard;

    vec3 viewD = normalize(viewPos - worldPos);
    vec3 lightSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);

    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1)
        {
            vec3 lightDirection = vec3(0.0);
            if (lights[i].type == LIGHT_DIRECTIONAL)
                lightDirection = -normalize(lights[i].target - lights[i].position);
            else if (lights[i].type == LIGHT_POINT)
                lightDirection = normalize(lights[i].position - worldPos);

            float NdotL = max(dot(vtxNormal, lightDirection), 0.0);
            lightSum += lights[i].color.rgb*NdotL;

            if (NdotL > 0.0)
            {
                vec3 halfVector = normalize(lightDirection + viewD);
                specularSum += pow(max(dot(vtxNormal, halfVector), 0.0), materialShininess);
            }
        }
    }

    vec4 color = (texel*((colDiffuse + vec4(specularSum, 1.0))*vec4(lightSum, 1.0)))
        + ambient*colDiffuse;
    gl_FragColor = pow(color, vec4(1.0/1.2));
}
