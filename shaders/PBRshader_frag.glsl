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

uniform vec3 albedo;
uniform float metallic;
uniform float roughness;
uniform float ao;

uniform vec3 cameraPos;
uniform vec3 lightPos;
uniform vec3 lightColor;



const float PI = 3.14159265359;

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
	// These two code lines were copied from "source"
	// calculate reflectance at normal incidence; if dia-electric (like plastic) use F0 
    // of 0.04 and if it's a metal, use the albedo color as F0 (metallic workflow)    
    vec3 F0 = vec3(0.04); 
    F0 = mix(F0, albedo, metallic);

	vec3 color = vec3(0.0f);
	for (int i = 0; i < 8; i++){
		vec3 normal = normalize(fragNormal);
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
	vec3 ambient = vec3(0.03) * albedo;
	color += ambient;
	// Copied from source:
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	fragColor = vec4(color, 1.0);
	return;



	// Calculate the D, G and F parts 
	vec3 normal = normalize(fragNormal);
	vec3 viewDir = normalize(cameraPos - fragPosition);
	vec3 lightDir = normalize(lightPos - fragPosition);
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

	vec3 radiance = lightColor / (length(lightPos - fragPosition) * length(lightPos - fragPosition));
	color = (kd * albedo / PI + BRDF) * radiance * max(dot(normal, lightDir), 0.0);
	color *= 4;
	// Copied from source:
	color = color / (color + vec3(1.0));
	color = pow(color, vec3(1.0/2.2));

	fragColor = vec4(color, 1.0);
}
