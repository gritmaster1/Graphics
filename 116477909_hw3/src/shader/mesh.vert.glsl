#version 410 core

// The "a" prefix stands for "attribute".
layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColor;

// These out variables will be passed along the pipeline
// and be refered with a uniform name in all shader stages,
// thus we add an "our" prefix.
out vec3 ourFragPos;
out vec3 ourNormal;
out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;

out vec4 LightColor;


void main()
{

    gl_Position = projection * view * model * vec4(aPosition, 1.0f);
    ourFragPos = vec3(model * vec4(aPosition, 1.0f));
    ourNormal = vec3(transpose(inverse(model)) * vec4(aNormal, 1.0f));
    ourColor = aColor;





    
    // ambient
    float ambientStrength = 0.1f;
    vec3 ambient = ambientStrength * lightColor;

    // diffuse
    vec3 norm = normalize(ourNormal);
    vec3 lightDir = normalize(lightPos - ourFragPos);
    float diff = max(dot(norm, lightDir), 0.0f);
    vec3 diffuse = diff * lightColor;

    // specular
    float specularStrength = 1.5f;
    vec3 viewDir = normalize(viewPos - ourFragPos);
    vec3 reflectDir = reflect(-lightDir, norm);
    float spec = pow(max(dot(viewDir, reflectDir), 0.0f), 32);
    vec3 specular = specularStrength * spec * lightColor;


    vec4 fragColor = vec4((ambient + diffuse + specular) * aColor, 1.0f);



    LightColor = fragColor;
}
