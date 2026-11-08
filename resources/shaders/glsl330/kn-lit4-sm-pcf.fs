#version 330

// This shader is based on the basic lighting shader
// This only supports one light, which is directional, and it (of course) supports shadows

// Input vertex attributes (from vertex shader)
in vec3 fragPosition;
in vec2 fragTexCoord;
//in vec4 fragColor;
in vec3 fragNormal;

// Input uniform values from raylib
uniform sampler2D texture0;
uniform vec4 colDiffuse;
// // alpha test value (0 = disabled, 1 = enabled)
uniform int alphaTest;

// Output fragment color
out vec4 finalColor;

// Input lighting values
uniform vec3 lightDir;
//uniform vec4 lightColor;


#define     MAX_LIGHTS              4
#define     LIGHT_DIRECTIONAL       0
#define     LIGHT_POINT             1

struct Light {
    int enabled;
    int type;
    vec3 position;  //position for point and directional light
    vec3 target;   //target(lookat) for directional light
    vec4 color;   //light color
    float attn_const;  //constant attenuation
    float attn_linear;   //linear attenuation
    float attn_quad; //quadratic attenuation
};

// Input lighting values
uniform Light lights[MAX_LIGHTS];

uniform vec4 ambient;
// access raylib's SHADER_LOC_VECTOR_VIEW 
uniform vec3 viewPos;
uniform float materialShininess; // Material shininess factor

// Input shadowmapping values
uniform mat4 lightVP; // Light source view-projection matrix

uniform sampler2D shadowMap;

uniform int shadowMapResolution;

uniform int receiveShadow;

void main()
{
    // Texel color fetching from texture sampler
    vec4 texelColor = texture(texture0, fragTexCoord);

    if (alphaTest > 0 && texelColor.a < 0.51) {
        discard; // Discard the fragment if alpha is below threshold
    }

    vec3 viewD = normalize(viewPos - fragPosition);
    vec3 lightSum = vec3(0.0);
    vec3 specularSum = vec3(0.0);

    //Loop through all enabled lights to calculate lighting
    for (int i = 0; i < MAX_LIGHTS; i++)
    {
        if (lights[i].enabled == 1) // Check if the light is enabled
        {
            vec3 light = vec3(0.0);

            if (lights[i].type == LIGHT_DIRECTIONAL)
            {
                light = -normalize(lights[i].target - lights[i].position);

                // Calculate diffuse component (Lambertian)
                // NdotL is max(0, dot(Normal, LightDirection))
                float NdotL = max(dot(fragNormal, light), 0.0);
                lightSum += lights[i].color.rgb * NdotL;

                // Calculate specular component (Blinn-Phong)
                float specCo = 0.0;
                if (NdotL > 0.0) // Only calculate specular if diffuse light is present
                {
                    // Blinn-Phong half-vector: normalized vector exactly halfway between view direction and light direction
                    // This is different from the Phong model which uses the reflection vector in default raylib light shader
                    vec3 halfVector = normalize(light + viewD);

                    // NdotH is max(0, dot(Normal, HalfVector))
                    float NdotH = max(dot(fragNormal, halfVector), 0.0);

                    // Raise NdotH to the power of the material's shininess
                    specCo = pow(NdotH, materialShininess);
                }
                specularSum += specCo; // Accumulate specular contribution

                //If this object does not receive shadows, skip shadow calculations
                if (receiveShadow == 1 && i==0)
                {      
                    vec3 normal = normalize(fragNormal);
                    vec3 l = -lightDir;

                    // Shadow calculations
                    vec4 fragPosLightSpace = lightVP * vec4(fragPosition, 1);
                    fragPosLightSpace.xyz /= fragPosLightSpace.w; // Perform the perspective division
                    fragPosLightSpace.xyz = (fragPosLightSpace.xyz + 1.0f) / 2.0f; // Transform from [-1, 1] range to [0, 1] range
                    vec2 sampleCoords = fragPosLightSpace.xy;
                    float curDepth = fragPosLightSpace.z;
                    // Slope-scale depth bias: depth biasing reduces "shadow acne" artifacts, where dark stripes appear all over the scene.
                    // The solution is adding a small bias to the depth
                    // In this case, the bias is proportional to the slope of the surface, relative to the light
                    // A large slope bias erases nearby caster shadows on inclined
                    // terrain (peter-panning). Keep enough bias to suppress acne
                    // without detaching the character's lower-body shadow.
                    float bias = max(0.0002f * (1.0 - dot(normal, l)), 0.00002f) + 0.00001f;
                    int shadowCounter = 0;
                    const int numSamples = 9;
                    // PCF (percentage-closer filtering) algorithm:
                    // Instead of testing if just one point is closer to the current point,
                    // we test the surrounding points as well.
                    // This blurs shadow edges, hiding aliasing artifacts.
                    vec2 texelSize = vec2(1.0f / float(shadowMapResolution));
                    for (int x = -1; x <= 1; x++)
                    {
                        for (int y = -1; y <= 1; y++)
                        {
                            float sampleDepth = texture(shadowMap, sampleCoords + texelSize * vec2(x, y)).r;
                            if (curDepth - bias > sampleDepth)
                            {
                                shadowCounter++;
                            }
                        }
                    }
                    lightSum = mix(lightSum, lightSum*ambient.rgb, float(shadowCounter) / float(numSamples));
                }

            }

            if (lights[i].type == LIGHT_POINT)
            {
                light = normalize(lights[i].position - fragPosition);

                // Calculate diffuse shading (Lambertian reflection)
                float diff = max(dot(fragNormal, light), 0.0);

                // Calculate attenuation (inverse square law)
                float distance = length(lights[i].position - fragPosition);
                float attenuation = 1.0 / (lights[i].attn_const + lights[i].attn_linear * distance + lights[i].attn_quad * (distance * distance));

                // Combine light color with attenuation and diffuse component
                vec3 lighting = (lights[i].color * diff).rgb * attenuation;

                // Apply lighting to the base color
                lightSum += colDiffuse.rgb * lighting;
            }
        }
    }

    finalColor = (texelColor*((colDiffuse + vec4(specularSum, 1.0))*vec4(lightSum, 1.0)));  
    
    // Gamma correction
    finalColor += ambient*colDiffuse;
    //finalColor = pow(finalColor, vec4(1.0/1.2));
}
