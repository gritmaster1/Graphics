#version 410 core

layout (quads, equal_spacing, ccw) in;

out vec3 ourNormal;
out vec3 ourFragPos;
out vec3 ourColor;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

uniform vec3 center;
uniform float radius;
uniform float minorradius; //for toruses
uniform vec3 color;
uniform int shapeType;  // 0 = sphere, 1 = cylinder, 2 = cone

const float kPi = 3.14159265358979323846f;

float a_x = 0.5f, a_y = 0.5f, a_z = 0.50f;
float n1 = 1.0f, n2 = 1.0f;
float v_max = 1.250f;

in vec3 tessControlPosition[];

float power(float base, float exp)
{
    return sign(base) * pow(abs(base), exp);
}


void main()
{
    vec4 WC = gl_in[0].gl_Position;

    // Parametric coordinates
    float u = gl_TessCoord.x;
    float v = gl_TessCoord.y;

    vec3 pos;

    float theta = (gl_TessCoord.x * 2.0 * kPi);
    float t = gl_TessCoord.y;  // Height along the cylinder (0 to 1)


    if (shapeType == 0) 
    { 
        float phi = 2.0f * kPi * u;  
        float theta2 = kPi * v;       

        pos = center + vec3(radius * sin(theta2) * cos(phi), radius * sin(theta2) * sin(phi), radius * cos(theta2));
    }
    else if (shapeType == 1)
    { 
        if(t == 0)
        {
             
            float h = 2.0;  // Height of the cylinder
            pos.x = radius * cos(theta);  // X position
            pos.z = radius * sin(theta);  // Y position
            pos.y = -h / 2.0f; 
        }
        else if(t == 1)
        {
             
            float h = 2.0;  // Height of the cylinder
            pos.x = radius * cos(theta);  // X position
            pos.z = radius * sin(theta);  // Y position
            pos.y = h / 2.0f; 
        }
        else
        {
  
            float h = 2.0;  // Height of the cylinder
            pos.x = radius * cos(theta);  // X position
            pos.z = radius * sin(theta);  // Y position
            pos.y = h * (t - 0.5); 
        }

        pos += center;
    }
    else if (shapeType == 2) 
    {
        float r = (1.0f - u) * radius;  

       {
  
            float h = 2.0;  // Height of the cylinder
            pos.x = (radius * (1 - t)) * cos(theta);  // X position
            pos.z = (radius * (1 - t)) * sin(theta);  // Y position
            pos.y = h * t; 
        }

        pos += center;
    }
    else if (shapeType == 3)
    {
        float Xangle = gl_TessCoord.x * 2.0f * kPi;
        float Zangle = gl_TessCoord.y * 2.0f * kPi;

        pos.x = (radius + minorradius * cos(Zangle)) * cos(Xangle);
        pos.y = (radius + minorradius * cos(Zangle)) * sin(Xangle);
        pos.z = minorradius * sin(Zangle);
   
        pos += center; 
    }
    else if (shapeType == 4)
    {
        float Su = mix(0.0, 2.0 * kPi, gl_TessCoord.x);
        float Sv = mix(-v_max, v_max, gl_TessCoord.y);

        // Evaluate parametric equations
        pos.x = a_x * power(cosh(Sv), n1) * power(cos(Su), n2);
        pos.z = a_y * power(cosh(Sv), n1) * power(sin(Su), n2);
        pos.y = a_z * power(sinh(Sv), n1);

        pos += center;

    }
    else if (shapeType == 5)
    {
        // Pentagonal face vertices
        vec3 p0 = tessControlPosition[0];
        vec3 p1 = tessControlPosition[1];
        vec3 p2 = tessControlPosition[2];
        vec3 p3 = tessControlPosition[3];
        vec3 p4 = tessControlPosition[4];

        // Barycentric coordinates for interpolation
        float u = gl_TessCoord.x;
        float v = gl_TessCoord.y;
        float w = 1.0 - u - v;
        
        // Interpolate across the pentagonal face
        pos = normalize(u * p0 + v * p1 + w * p2 + (1.0 - u - v - w) * p3 + p4);
        
        // Refine by projecting onto a sphere of radius 1
        vec3 refinedPosition = normalize(pos);
        
        pos = refinedPosition + center;
    }

    gl_Position = projection * view * model * vec4(pos, 1.0f);

    ourFragPos = vec3(model * vec4(pos, 1.0f));

    ourNormal = normalize(pos - center);

    ourColor = color;
}
