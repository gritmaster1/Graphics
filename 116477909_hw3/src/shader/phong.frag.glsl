#version 410 core

in vec3 ourFragPos;
in vec3 ourNormal;
in vec3 ourColor;
in vec4 LightColor;

out vec4 fragColor;

uniform vec3 viewPos;
uniform vec3 lightPos;
uniform vec3 lightColor;


uniform int displayMode;


void main()
{



    if(displayMode == 1)
    {
        fragColor = vec4(LightColor);
    }
    else if (displayMode == 2)
    {
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


        fragColor = vec4((ambient + diffuse + specular) * norm, 1.0f);
    }
    else
    {

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


        fragColor = vec4((ambient + diffuse + specular) * ourColor, 1.0f);
    }


}
