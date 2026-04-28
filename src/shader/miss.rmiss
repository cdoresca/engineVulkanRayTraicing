#version 460
#extension GL_EXT_ray_tracing : require
#include "struct.glsl"
layout(location = 0) rayPayloadInEXT Hit payload;


const float PI = 3.1415926535;
const vec3 betaRayleigh = vec3(5.8e-6, 13.5e-6, 33.1e-6);
const float betaMie = 21e-6;
const float g = 0.76;

const float planetRadius = 6360e3;
const float atmosphereRadius = 6460e3;

const float Hr = 8000.0;   // Rayleigh scale height
const float Hm = 1200.0;   // Mie scale height

float phaseMie(float cosTheta) {
    return (3.0 / (8.0 * PI)) *
           ((1.0 - g * g) * (1.0 + cosTheta * cosTheta)) /
           ((2.0 + g * g) * pow(1.0 + g * g - 2.0 * g * cosTheta, 1.5));
}


float phaseRayleigh(float cosTheta) {
    return (3.0 / (16.0 * PI)) * (1.0 + cosTheta * cosTheta);
}

void main()
{
    vec3 sunDirection = normalize(vec3(0.0, 1.0, 0.0));
    float sunIntensity = 20.0;

    vec3 rayDir = normalize(gl_WorldRayDirectionEXT);
    vec3 rayOrigin = vec3(0.0, planetRadius + 1.0, 0.0);

    float tMax = 120000.0;   // march distance
    int steps = 16;
    float dt = tMax / float(steps);

    vec3 totalRayleigh = vec3(0.0);
    vec3 totalMie = vec3(0.0);

    float opticalDepthR = 0.0;
    float opticalDepthM = 0.0;

    for (int i = 0; i < steps; i++)
    {
        float t = dt * (float(i) + 0.5);
        vec3 pos = rayOrigin + rayDir * t;
        float height = length(pos) - planetRadius;

        float densityR = exp(-height / Hr);
        float densityM = exp(-height / Hm);

        opticalDepthR += densityR * dt;
        opticalDepthM += densityM * dt;

        float cosTheta = dot(rayDir, sunDirection);
        float pr = phaseRayleigh(cosTheta);
        float pm = phaseMie(cosTheta);

        vec3 attenuation = exp(
            -(betaRayleigh * opticalDepthR +
              vec3(betaMie) * opticalDepthM)
        );

        totalRayleigh += densityR * pr * attenuation * dt;
        totalMie      += densityM * pm * attenuation * dt;
    }

    vec3 sky =
        sunIntensity *
        (betaRayleigh * totalRayleigh +
         vec3(betaMie) * totalMie);

    sky = sky / (sky + vec3(1.0)); // tonemap

    payload.color = sky;
    
}