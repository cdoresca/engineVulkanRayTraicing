
const float PI = 3.1415926535897932384626433832795;

uint LCGStep(inout uint z, uint A, uint C)
{
  return z = (A * z + C);
}

uint TausStep(inout uint z, int S1, int S2, int S3, uint M)
{
  uint b = (((z << S1) ^ z) >> S2);
  return z = (((z & M) << S3) ^ b);
}

uint z1, z2, z3, z4;

void initSeeds(uvec2 pixelPos, uvec2 dim, uint frameIndex)
{
    uint pixelId = pixelPos.y * dim.x + pixelPos.x;
    
    z1 = pixelId * 1973u  + frameIndex * 9277u  + 26699u;
    z2 = pixelId * 4933u  + frameIndex * 2699u  + 51331u;
    z3 = pixelId * 9781u  + frameIndex * 3571u  + 93913u;
    z4 = pixelId * 6271u  + frameIndex * 7699u  + 10007u;
}
float HybridTaus()
{
  // Combined period is lcm(p1,p2,p3,p4)~ 2^121
   return 2.3283064365387e-10 * (              // Periods
    TausStep(z1, 13, 19, 12, 4294967294u) ^  // p1=2^31-1
    TausStep(z2, 2, 25, 4, 4294967288u) ^    // p2=2^30-1
    TausStep(z3, 3, 11, 17, 4294967280u) ^   // p3=2^28-1
    LCGStep(z4, 1664525, 1013904223u)        // p4=2^32
   );
}

 vec3 random_cosine_direction() {
    float r1 = HybridTaus();
    float r2 = HybridTaus();

    float phi = 2*PI*r1;
    float x = cos(phi) * sqrt(r2);
    float y = sin(phi) * sqrt(r2);
    float z = sqrt(1-r2);

    return vec3(x, y, z);
}

float degreeToRadians(float x){
    return x * PI / 180.0;
}