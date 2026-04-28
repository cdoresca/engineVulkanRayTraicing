#version 460
#extension GL_EXT_ray_tracing : require

#include "struct.glsl"
layout(location = 1) rayPayloadInEXT bool shadow;



void main()
{
    shadow = false;
    

}