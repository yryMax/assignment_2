#version 410

uniform mat4 mvpMatrix;
uniform mat4 modelMatrix;
uniform mat3 normalModelMatrix;


layout(location = 0) in vec3 position;
layout(location = 1) in vec3 normal;
layout(location = 2) in vec2 texCoord;
layout(location = 3) in vec3 tangent;
layout(location = 4) in vec3 bitangent;

out vec3 fragPosition;
out vec3 fragNormal;
out vec2 fragTexCoord;
out mat3 TBN;

void main()
{
    gl_Position = mvpMatrix * vec4(position, 1.0);

    fragPosition = (modelMatrix * vec4(position, 1.0)).xyz;
    fragTexCoord = texCoord;


    vec3 T = normalize(normalModelMatrix * tangent);
    vec3 B = normalize(normalModelMatrix * bitangent);
    vec3 N = normalize(normalModelMatrix * normal);

    TBN = mat3(T, B, N);


    fragNormal = normalize(normalModelMatrix * normal);

}
