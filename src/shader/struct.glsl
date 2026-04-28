
struct Material {
    vec4 diffuse;
    vec4 emissive;
};

struct Vertex {
    vec4 pos;
    vec3 normal;
    uint mat_id;
   vec2 uv;
   vec2 pad3;
};

struct PointLight{
    vec3 position;
    float intensity;
};

struct DirectionalLight {                 
    vec3 direction;   
    float intensity;
};

struct Hit{
    vec3 color;
    int depth;
    
};