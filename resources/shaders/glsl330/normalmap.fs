#version 330 core

// Interpolated values from the vertex shaders
in vec4 vcolor;
in vec2 UV;
in vec3 Position_worldspace;
in vec3 EyeDirection_cameraspace;
in vec3 LightDirection_cameraspace;
in vec3 LightDirection_tangentspace;
in vec3 EyeDirection_tangentspace;

// Output data
out vec4 color;

// Values that stay constant for the whole mesh.
uniform sampler2D texture0;
uniform sampler2D texture2;
uniform mat4 matView;
uniform mat4 matModel;
uniform vec3 LightPosition_worldspace;
uniform vec3 LightColor;
uniform float LightPower;

void main(){
	
	// Material properties
	vec3 MaterialDiffuseColor = texture( texture0, UV ).rgb;
	vec3 MaterialAmbientColor = vec3(0.1,0.1,0.1) * MaterialDiffuseColor;
	// Local normal, in tangent space. V tex coordinate is inverted because normal map is in TGA (not in DDS) for better quality
	vec3 TextureNormal_tangentspace = normalize(texture( texture2, vec2(UV.x, UV.y) ).rgb*2.0 - 1.0);
	
	// Distance to the light
	float distance = length( LightPosition_worldspace - Position_worldspace );

	// Normal of the computed fragment, in camera space
	vec3 n = TextureNormal_tangentspace;
	// Direction of the light (from the fragment to the light)
	vec3 l = normalize(LightDirection_tangentspace);
	// Cosine of the angle between the normal and the light direction, 
	// clamped above 0
	//  - light is at the vertical of the triangle -> 1
	//  - light is perpendicular to the triangle -> 0
	//  - light is behind the triangle -> 0
	float cosTheta = clamp( dot( n,l ), 0,1 );

	// Eye vector (towards the camera)
	vec3 E = normalize(EyeDirection_tangentspace);
	// Direction in which the triangle reflects the light
	vec3 R = reflect(-l,n);
	// Cosine of the angle between the Eye vector and the Reflect vector,
	// clamped to 0
	//  - Looking into the reflection -> 1
	//  - Looking elsewhere -> < 1
	float cosAlpha = clamp( dot( E,R ), 0,1 );
	
	color = 
		// Ambient : simulates indirect lighting
		vec4(MaterialAmbientColor,1) +
		// Diffuse : "color" of the object
		vcolor * vec4(MaterialDiffuseColor * LightColor * LightPower * cosTheta / (distance*distance),1);
		// Specular : reflective highlight, like a mirror
		//vec4(MaterialSpecularColor * LightColor * LightPower * pow(cosAlpha,5) / (distance*distance),1);

}
