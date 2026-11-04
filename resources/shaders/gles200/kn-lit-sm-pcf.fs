#version 100
precision mediump float;
precision mediump int;
varying vec3 fragPosition;
varying vec2 fragTexCoord;
varying vec3 fragNormal;
uniform sampler2D texture0;
uniform vec4 colDiffuse;
uniform int alphaTest;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 ambient;
uniform vec3 viewPos;
uniform mat4 lightVP;
uniform sampler2D shadowMap;
uniform int shadowMapResolution;
uniform int receiveShadow;
void main() {
    vec4 texel = texture2D(texture0, fragTexCoord);
    if (alphaTest > 0 && texel.a < 0.51) discard;
    vec3 normal = normalize(fragNormal);
    vec3 lightVector = -lightDir;
    float diffuse = max(dot(normal, lightVector), 0.0);
    vec3 viewD = normalize(viewPos - fragPosition);
    float specular = diffuse > 0.0 ? pow(max(dot(viewD, reflect(-lightVector, normal)), 0.0), 16.0) : 0.0;
    vec4 color = texel*((colDiffuse + vec4(vec3(specular), 1.0))*vec4(lightColor.rgb*diffuse, 1.0));
    if (receiveShadow == 1) {
        vec4 lightSpace = lightVP*vec4(fragPosition, 1.0);
        vec3 projected = lightSpace.xyz/lightSpace.w*0.5 + 0.5;
        float bias = max(0.001*(1.0 - dot(normal, lightVector)), 0.0005) + 0.00005;
        float shadow = 0.0;
        vec2 texelSize = vec2(1.0/float(shadowMapResolution));
        for (int x = -1; x <= 1; x++) for (int y = -1; y <= 1; y++)
            shadow += projected.z - bias > texture2D(shadowMap, projected.xy + texelSize*vec2(float(x), float(y))).r ? 1.0 : 0.0;
        color = mix(color, color*ambient, shadow/9.0);
    }
    gl_FragColor = pow(color, vec4(1.0/1.2));
}
