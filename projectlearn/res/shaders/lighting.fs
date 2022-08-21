#version 330 core
out vec4 FragColor;

// struct Material {
//     vec3 ambient;
//     vec3 diffuse;
//     vec3 specular;    
//     float shininess;
// };

struct Material{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
};

struct Light {
    vec3 position;

    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform Light light;
uniform Light bulbs[5];
uniform bool hasBulbs;
uniform bool isBulb;
uniform bool isGlass;

uniform sampler2D texture_diffuse1;

void main()
{
    // ambient
    vec3 ambient = vec3(light.ambient) * material.ambient.rgb;
    float attenuationFactor = 0.5;
  	
    // diffuse 
    vec3 norm = normalize(Normal);
    vec3 lightDir = normalize(light.position - FragPos);
    float diff = max(dot(norm, lightDir), 0.0);
    vec3 totalDiffuse = vec3(light.diffuse) * (diff * material.diffuse.rgb);

    for( int i=0; hasBulbs && i<5; ++i )
    {
        lightDir = normalize(bulbs[i].position - FragPos);
        diff = max(dot(norm, lightDir), 0.0);
        totalDiffuse += attenuationFactor * vec3(bulbs[i].diffuse) * (diff*material.diffuse.rgb);
    }
    
    // specular
    vec3 viewDir = normalize(viewPos - FragPos);
    lightDir = normalize(light.position - FragPos);
    vec3 reflectDir = reflect(-lightDir, norm);  
    float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    vec3 totalSpecular = vec3(light.specular) * (spec * material.specular.rgb);

    for( int i=0; hasBulbs && i<5; ++i )
    {
        lightDir = normalize(bulbs[i].position - FragPos);
        reflectDir = reflect(-lightDir, norm);  
        spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
        totalSpecular += attenuationFactor * vec3(bulbs[i].specular) * (spec * material.specular.rgb);
    }

    vec3 result = ambient + totalDiffuse;
    if( isBulb )
    {
        result = vec3(1,1,1);
    }
    // FragColor = vec4(result, 1.0);
    if(isGlass)FragColor = vec4(result,0.5);
    else FragColor = vec4(result,1.0);
    // FragColor = texture(texture_diffuse1,TexCoords) * vec4(result,1.0);
} 