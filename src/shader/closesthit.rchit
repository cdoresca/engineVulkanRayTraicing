#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : require
#extension GL_GOOGLE_include_directive : require

#include "generatorRandom.glsl"
#include "struct.glsl"


layout(location = 0) rayPayloadInEXT Hit payload;

layout(location = 1) rayPayloadEXT bool shadow;



layout(push_constant, std430) uniform camera{
    vec4 pos;
    vec4 lookAt;
    vec4 up;
    float fov;
};

layout(set = 0, binding = 0) uniform accelerationStructureEXT topLevelAS;

layout(set = 1, binding = 0) readonly buffer VertexBuffer {
    Vertex vertices[];
} vertexBuffers[];  

layout(set = 1, binding = 1) readonly buffer IndexBuffer {
    uint indices[];
} indexBuffers[]; 

layout(set = 2, binding = 0) buffer MaterialBuffer {
    Material color;
}materials[];

hitAttributeEXT vec2 bary;



void main()
{
    uint meshIdx = gl_InstanceCustomIndexEXT;
    uint primID = gl_PrimitiveID;


    uint i0 = indexBuffers[nonuniformEXT(meshIdx)].indices[gl_PrimitiveID * 3];
    uint i1 = indexBuffers[nonuniformEXT(meshIdx)].indices[gl_PrimitiveID * 3 + 1];
    uint i2 = indexBuffers[nonuniformEXT(meshIdx)].indices[gl_PrimitiveID * 3 + 2];

    Vertex v0 = vertexBuffers[nonuniformEXT(meshIdx)].vertices[i0];
    Vertex v1 = vertexBuffers[nonuniformEXT(meshIdx)].vertices[i1];
    Vertex v2 = vertexBuffers[nonuniformEXT(meshIdx)].vertices[i2];

    uint matId = v0.mat_id;
    Material mat = materials[nonuniformEXT(matId)].color ;

    if(mat.emissive.w > 0.0) {
        payload.color = mat.emissive.xyz * mat.emissive.w;
        return;
    }

    float w0 = 1.0 - bary.x - bary.y;
    float w1 = bary.x;
    float w2 = bary.y;

    vec3 N = normalize(
        v0.normal * w0 +
        v1.normal * w1 +
        v2.normal * w2
    );

    vec3 albedo = mat.diffuse.xyz;
    vec3 P = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
  
    PointLight light;
    light.position = pos.xyz;
    light.intensity = 1.0;
    vec3 L = light.position - P;
    float distToLight = length(L);
    vec3 Ldir = normalize(L);
    
   
    float k = 0.8;
    vec3 brdf = k * albedo / PI;

    // shadow ray
    shadow = true;

    uint rayFlags =
        gl_RayFlagsTerminateOnFirstHitEXT |
        gl_RayFlagsOpaqueEXT |
        gl_RayFlagsSkipClosestHitShaderEXT;

    traceRayEXT(
        topLevelAS,
        rayFlags,
        0xFF,
        0,
        0,
        1,
        P + N * 0.001,
        0.0,
        Ldir,
        distToLight,
        1
    );

    float NdotL = max(dot(N, Ldir), 0.0);

    vec3 lighting = vec3(0.0);

    if(!shadow)
    {
        lighting = brdf * light.intensity * NdotL;
    }

    payload.color = lighting;
}
