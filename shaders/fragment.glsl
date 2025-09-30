#version 330 core
out vec4 FragColor;

in vec3 FragPos;
in vec3 Normal;

uniform vec3 viewPos;
uniform vec3 lightPositions[4];
uniform vec3 lightColors[4];
uniform vec3 objectColor;

void main()
{
    vec3 norm = normalize(Normal);
    vec3 viewDir = normalize(viewPos - FragPos);
    
    vec3 result = vec3(0.0);
    
    // Loop through all lights
    for(int i = 0; i < 4; i++)
    {
        // Ambient
        vec3 ambient = 0.1 * lightColors[i];
        
        // Diffuse
        vec3 lightDir = normalize(lightPositions[i] - FragPos);
        float diff = max(dot(norm, lightDir), 0.0);
        vec3 diffuse = diff * lightColors[i];
        
        // Specular
        vec3 reflectDir = reflect(-lightDir, norm);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);
        vec3 specular = 0.5 * spec * lightColors[i];
        
        result += (ambient + diffuse + specular);
    }
    
    FragColor = vec4(result * objectColor, 1.0);
}