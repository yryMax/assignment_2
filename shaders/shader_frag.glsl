#version 410

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

uniform sampler2D colorMap;
uniform bool hasTexCoords;
uniform bool useMaterial;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;

layout(location = 0) out vec4 fragColor;



void main()
{
	if (useMaterial) {
		fragColor = texture(colorMap, fragTexCoord);
		//fragColor = vec4(1,1,0,1);
		return;
	}
    fragColor = vec4(0.8, 0.8, 0.8, 1);
}
