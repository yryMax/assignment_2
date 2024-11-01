#version 410

layout(std140) uniform Material // Must match the GPUMaterial defined in src/mesh.h
{
    vec3 kd;
	vec3 ks;
	float shininess;
	float transparency;
};

uniform sampler2D colorMap;
uniform sampler2D normalMap;
uniform int useNormalMapping;
uniform int useTexture;
uniform int useEnvMap;
uniform samplerCube envMap;
uniform vec3 cameraPos;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in mat3 TBN;

layout(location = 0) out vec4 fragColor;

void main()
{
    vec3 normal = normalize(fragNormal);
    vec2 texCoord = fragTexCoord; // Repeat texture
    vec3 basec = kd;
    if (useTexture == 1)
    {
        basec = texture(colorMap, texCoord).rgb;
    }
    vec3 normalMapColor = texture(normalMap, texCoord).rgb;
    if (useNormalMapping == 1)
    {
        normal = normalize(TBN * (2.0 * normalMapColor - 1.0));
    }


    if (useEnvMap == 1)
    {
        vec3 I = normalize(fragPosition - cameraPos);
        vec3 R = normalize(reflect(I, normal));
        vec3 envColor = texture(envMap, R).rgb;
        fragColor = vec4(envColor, 1);
        return;
    }

    // phong shading
    vec3 lightDir = normalize(vec3(1, -1, 0));
    float diff = max(dot(normal, lightDir), 0.0);
    vec3 diffuse = basec * diff;
    vec3 ambient = basec * 0.6;
    fragColor = vec4(diffuse + ambient, 1);


}
