#version 460
#extension GL_EXT_ray_tracing : require
#extension GL_EXT_nonuniform_qualifier : require

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

    
    
   
    float w0 = 1.0 - bary.x - bary.y;
    float w1 = bary.x;
    float w2 = bary.y;

    vec3 N = normalize(
        v0.normal * w0 +
        v1.normal * w1 +
        v2.normal * w2
    );
    vec3 P = gl_WorldRayOriginEXT + gl_WorldRayDirectionEXT * gl_HitTEXT;
    vec3 up = abs(N.z) < 0.999 ? vec3(0,0,1) : vec3(1,0,0);
    vec3 tangent   = normalize(cross(up, N));
    vec3 bitangent = cross(N, tangent);

    uint rayFlags =
        gl_RayFlagsTerminateOnFirstHitEXT |
        gl_RayFlagsOpaqueEXT |
        gl_RayFlagsSkipClosestHitShaderEXT;
   
    uint totalLight = 0;
    uint numRay  = 8;
   
   
    for(int i = 0; i < numRay; i++){
        shadow = true;
        vec3 localDir = random_cosine_direction();

    // Transformer vers world space
        vec3 worldDir = localDir.x * tangent +
                    localDir.y * bitangent +
                    localDir.z * N;

        traceRayEXT(
            topLevelAS,
            rayFlags,
            0xFF,
            0,
            0,
            1,
            P + N * 0.001,
            0.0,
             worldDir,
            1,
            1
        );
        if(!shadow) ++totalLight;
         
    
    }

        payload.color = vec3(float(totalLight) / float(numRay));
    }