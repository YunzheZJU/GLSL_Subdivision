#version 430

//struct LightInfo {
//    vec4 Position;  // Light position in eye coords.
//    vec3 Intensity; // A,D,S intensity
//};
//uniform LightInfo Light;
//
//struct MaterialInfo {
//    vec3 Ka;            // Ambient reflectivity
//    vec3 Kd;            // Diffuse reflectivity
//    vec3 Ks;            // Specular reflectivity
//    float Shininess;    // Specular shininess factor
//};
//uniform MaterialInfo Material;

uniform float LineWidth;
uniform vec4 LineColor;
uniform vec4 QuadColor;

noperspective in vec3 EdgeDistance;

layout(location = 0) out vec4 FragColor;

float edgeMix() {
    float d = min(min(EdgeDistance.x, EdgeDistance.y), EdgeDistance.z);

    if (d < LineWidth - 1) {
        return 1.0;
    }
    else if (d > LineWidth + 1) {
        return 0.0;
    }
    else {
        float x = d - (LineWidth - 1);
        return exp2(-2.0 * (x * x));
    }
}

//vec3 phongModel(vec3 pos, vec3 norm) {
//    vec3 s = normalize(vec3(Light.Position) - pos);
//    vec3 v = normalize(-pos.xyz);
//    vec3 r = reflect(-s, norm);
//    vec3 ambient = Light.Intensity * Material.Ka;
//    float sDotN = max( dot(s,norm), 0.0 );
//    vec3 diffuse = Light.Intensity * Material.Kd * sDotN;
//    vec3 spec = vec3(0.0);
//    if( sDotN > 0.0 )
//        spec = Light.Intensity * Material.Ks * pow(max(dot(r, v), 0.0), Material.Shininess);
//    return ambient + diffuse + spec;
//}

void main() {
    float mixVal = edgeMix();
    FragColor = mix(QuadColor, LineColor, mixVal);
}