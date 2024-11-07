#version 410
// Code based on https://learnopengl.com/PBR/Theory and https://learnopengl.com/PBR/Lighting

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

uniform sampler2D roughnessMap;
uniform sampler2D ambientMap;
uniform sampler2D metallicMap;
uniform int usePBR;
uniform int useRoughness;
uniform int useAmbient;
uniform int useMetallic;

in vec3 fragPosition;
in vec3 fragNormal;
in vec2 fragTexCoord;
in mat3 TBN;


uniform vec3 albedo;
uniform float metallicInput;
uniform float roughnessInput;
uniform float ao;

uniform vec3 lightPos;
uniform vec3 lightColor;

const float PI = 3.14159265359;

layout(location = 0) out vec4 fragColor;


// Calculates the Normal distribution function also know as the Trowbridge-Reitz GGX
// This is the D part of the Cook-Torrance specular BRDF
float normalDistribution(vec3 normal, vec3 halfwayVector, float roughness){
	float roughness2 = roughness * roughness;
	float dotNormHalf = max(dot(normal, halfwayVector), 0.0);
	float intermediate = dotNormHalf * dotNormHalf * (roughness2 - 1) + 1;
	return roughness2/(PI * (intermediate * intermediate));
}

// Calculates the Geometry function also know as the Schlick-GGX
// This is the G part of the Cook-Torrance specular BRDF
float geometry(vec3 normal, vec3 viewDir, vec3 lightDir, float roughness){
	float dotNormalView = max(dot(normal, viewDir), 0.0);
	float dotNormalLight = max(dot(normal, lightDir), 0.0);
	roughness += 1;
	roughness = (roughness * roughness) / 8.0;
	float viewPart = dotNormalView/(dotNormalView * (1 - roughness) + roughness);
	float lightPart = dotNormalLight/(dotNormalLight * (1 - roughness) + roughness);
	return viewPart * lightPart;
}

// Calculates the Fresnel equation using the Fresnel-Schlick approximation
// This is the F part of the Cook-Torrance specular BRDF
vec3 fresnel(vec3 halfwayVector, vec3 viewDir, vec3 fresnelConstant){
	float dothalfView = clamp(dot(halfwayVector, viewDir), 0.0, 1.0);
	float intermediate = pow(clamp(1.0 - dothalfView, 0.0, 1.0), 5);
	return fresnelConstant + (1 - fresnelConstant) * intermediate;
}

void main()
{
    vec3 normal = normalize(fragNormal);
    vec2 texCoord = fragTexCoord; // Repeat texture
    vec3 basec = kd;
    float ambientOcclusion = 1.0f;
    vec3 albedo = albedo;
    vec3 ambient;
    float metallic = metallicInput;
    float roughness = roughnessInput;
    fragColor = vec4(0.0f);
    if (useTexture == 1)
    {
        basec = texture(colorMap, texCoord).rgb;
        albedo = basec;
    }
    if (useRoughness == 1)
    {
        roughness = texture(roughnessMap, texCoord).r;
    }
    if (useAmbient == 1)
    {
        ambientOcclusion = texture(ambientMap, texCoord).r;
        //ambient constant "0.03" copied from "https://learnopengl.com/PBR/Lighting"
        ambient = vec3(0.03) * albedo * ambientOcclusion;
    }
    if (useMetallic == 1)
    {
        metallic = texture(metallicMap, texCoord).r;
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
    }

    if (usePBR == 1){
        vec3 lightPos2[8] = {
            vec3(lightPos[0], lightPos[1], lightPos[2]),
            vec3(-lightPos[0], lightPos[1], lightPos[2]),
            vec3(-lightPos[0], -lightPos[1], lightPos[2]),
            vec3(lightPos[0], -lightPos[1], lightPos[2]),
		    vec3(lightPos[0], lightPos[1], -lightPos[2]),
            vec3(-lightPos[0], lightPos[1], -lightPos[2]),
            vec3(-lightPos[0], -lightPos[1], -lightPos[2]),
            vec3(lightPos[0], -lightPos[1], -lightPos[2]),
        };
	    vec3 lightColor2 = lightColor;

	    // These two code lines were copied from "https://learnopengl.com/PBR/Lighting"
	    // calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
        // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
        vec3 F0 = vec3(0.04); 
        F0 = mix(F0, albedo, metallic);

	    vec3 color = vec3(0.0f);
	    for (int i = 0; i < 8; i++){
		    vec3 viewDir = normalize(cameraPos - fragPosition);
		    vec3 lightDir = normalize(lightPos2[i] - fragPosition);
		    vec3 halfwayVector = normalize(lightDir + viewDir);
		    float D = normalDistribution(normal, halfwayVector, roughness);
		    float G = geometry(normal, viewDir, lightDir, roughness);
		    vec3 F = fresnel(halfwayVector, viewDir, F0);

		    // Calculate the BRDF
		    float intermediate = 4 * max(dot(normal, viewDir), 0.0) * max(dot(normal, lightDir), 0.0) + 0.001;
		    vec3 BRDF = D * G * F / intermediate;

		    vec3 ks = F;
		    vec3 kd = vec3(1.0f) - ks;
		    kd = kd * (1.0 - metallic);

		    vec3 radiance = lightColor2 / (length(lightPos2[i] - fragPosition) * length(lightPos2[i] - fragPosition));
		    color += (kd * albedo / PI + BRDF) * radiance * max(dot(normal, lightDir), 0.0);
	    }
	    color += ambient;
	    // These two code lines were copied from "https://learnopengl.com/PBR/Lighting"
	    color = color / (color + vec3(1.0));
	    color = pow(color, vec3(1.0/2.2));

	    fragColor += vec4(color, 1.0);
    }
    else if (useEnvMap != 1) {
        // phong shading
        vec3 lightDir = normalize(vec3(1, -1, 0));
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = albedo * diff;
        if (useAmbient != 1){
            vec3 ambient = albedo * 0.6;
        }
        fragColor = vec4(diffuse + ambient, 1);
    }
}
