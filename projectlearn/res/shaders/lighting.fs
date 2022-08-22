#version 330 core
out vec4 FragColor;

const int MAX_BULBS = 5;

struct Material{
    vec4 ambient;
    vec4 diffuse;
    vec4 specular;

    float shininess;
};

struct BaseLight {
    // vec3 Color;
    vec3 ambient;
    vec3 diffuse;
    vec3 specular;
};

struct SunLight {
    vec3 position;
    BaseLight base;
    // vec3 direction;
};

struct Attenuation{
    float constant;
    float linear;
    float exp;
};

struct PointLight{
    BaseLight base;
    vec3 position;
    Attenuation atten; 
};

struct SpotLight{
    PointLight base;
    vec3 direction;
    float cutoff;
};

in vec3 FragPos;  
in vec3 Normal;  
in vec2 TexCoords;

uniform vec3 viewPos;
uniform Material material;
uniform SunLight sunLight;
uniform SpotLight bulbs[MAX_BULBS];
uniform int numBulbs;
uniform bool isBulb;

uniform sampler2D texture_diffuse1;

vec4 CalcLightInternal(BaseLight light, vec3 LightDir, vec3 normal, bool bulb)
{
    // vec4 ambientColor = vec4(light.Color,1.0f) * light.ambient * material.ambient.rgba;
    vec4 ambientColor = vec4(light.ambient,1.0) * material.ambient.rgba;

    float diffuseFactor = dot(normal, LightDir);

    vec4 diffuseColor = vec4(0,0,0,0);
    vec4 specularColor = vec4(0,0,0,0);

    if (diffuseFactor > 0 )
    {
        // diffuseColor = vec4(light.Color, 1.0f) * light.diffuse * material.diffuse.rgba * diffuseFactor;
        diffuseColor = vec4(light.diffuse,1.0) * material.diffuse.rgba * diffuseFactor;

        vec3 viewDir = normalize(viewPos-FragPos);
        vec3 reflectDir = normalize(reflect(-LightDir, normal));
        float specularFactor = dot(viewDir,reflectDir);
        if(specularFactor>0)
        {
            float exp;
            if( bulb ) exp = 256.f;
            else exp = material.shininess;
            float spec = pow(specularFactor,exp);
            // specularColor = vec4(light.Color, 1.0f) * light.specular * material.specular.rgba * spec;
            specularColor = vec4(light.specular,1.0) * material.specular.rgba * spec;
        }
    }

    return (ambientColor+diffuseColor+specularColor);
}

vec4 CalcDirectionalLight( vec3 normal )
{
    vec3 dir = normalize(sunLight.position-FragPos);
    return CalcLightInternal(sunLight.base, dir, normal, false);
}

vec4 CalcPointLight(PointLight l, vec3 normal)
{
    vec3 LightDir = l.position - FragPos;
    float distance = length(LightDir);
    LightDir = normalize(LightDir);

    vec4 Color = CalcLightInternal(l.base, LightDir,normal, true);
    float attenuationFactor = l.atten.constant + (l.atten.linear * distance) + (l.atten.exp * distance * distance);

    return Color/attenuationFactor;
}

vec4 CalcSpotLight( SpotLight l, vec3 normal )
{
    vec3 LightDir = normalize(FragPos-l.base.position);
    float spotFactor = dot(LightDir, l.direction);

    if( spotFactor>l.cutoff ) 
    {
        vec4 Color = CalcPointLight(l.base, normal);
        float spotLightIntensity = 1.0-((1.0-spotFactor)/(1.0-l.cutoff));
        // return Color * spotLightIntensity;
        return Color;
    }
    else
    {
        return vec4(0,0,0,0);
    }

}

void main()
{

    vec3 normal = normalize(Normal);
    vec4 totalLight = CalcDirectionalLight(normal);

    for( int i=0; i<min(numBulbs,MAX_BULBS); ++i )
    {
        // totalLight += CalcPointLight(i,normal);
        totalLight += CalcSpotLight(bulbs[i],normal);
    }
    // // ambient
    // vec4 ambient = vec4(light.ambient,1.0) * material.ambient.rgba;
    // float attenuationFactor = 0.05;
  	
    // // diffuse 
    // vec3 norm = normalize(Normal);
    // vec3 lightDir = normalize(light.position - FragPos);
    // float diff = max(dot(norm, lightDir), 0.0);
    // vec4 totalDiffuse = vec4(light.diffuse,1.0) * (diff * material.diffuse.rgba);

    // for( int i=0; hasBulbs && i<5; ++i )
    // {
    //     lightDir = normalize(bulbs[i].position - FragPos);
    //     diff = max(dot(norm, lightDir), 0.0);
    //     totalDiffuse += attenuationFactor * vec4(bulbs[i].diffuse,1.0) * (diff*material.diffuse.rgba);
    // }
    
    // // specular
    // vec3 viewDir = normalize(viewPos - FragPos);
    // lightDir = normalize(light.position - FragPos);
    // vec3 reflectDir = reflect(-lightDir, norm);  
    // float spec = pow(max(dot(viewDir, reflectDir), 0.0), material.shininess);
    // vec4 totalSpecular = vec4(light.specular,1.0) * (spec * material.specular.rgba);

    // for( int i=0; hasBulbs && i<5; ++i )
    // {
    //     lightDir = normalize(bulbs[i].position - FragPos);
    //     reflectDir = reflect(-lightDir, norm);  
    //     spec = pow(max(dot(viewDir, reflectDir), 0.0), 256);
    //     totalSpecular += attenuationFactor * vec4(bulbs[i].specular,1.0) * (spec * material.specular.rgba);
    // }

    // vec4 result = ambient + totalDiffuse;
    if( isBulb )
    {
        totalLight = vec4(255,178,0,1) * totalLight;
    }
    // FragColor = vec4(result, 1.0);
    FragColor = totalLight;
    // FragColor = texture(texture_diffuse1,TexCoords) * vec4(result,1.0);
} 