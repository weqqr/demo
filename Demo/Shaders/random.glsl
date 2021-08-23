#ifndef RANDOM_GLSL
#define RANDOM_GLSL

#include "common.glsl"

#define RandomState uvec2

// https://prng.di.unimi.it/xoroshiro64star.c
uint xoroshiro64star_rotl(uint x, uint k)
{
    return (x << k) | (x >> (32 - k));
}

uint xoroshiro64star(inout RandomState s)
{
    uint result = s[0] * 0x9E3779BB;

    s[1] ^= s[0];
    s[0] = xoroshiro64star_rotl(s[0], 26) ^ s[1] ^ (s[1] << 9); // a, b
    s[1] = xoroshiro64star_rotl(s[1], 13); // c

    return result;
}

float bits_to_normalized_float(in uint bits)
{
    return uintBitsToFloat((bits & 0x007FFFFFu) | 0x3F800000u) - 1.0;
}

float random(inout RandomState state)
{
    return bits_to_normalized_float(xoroshiro64star(state));
}

// https://math.stackexchange.com/a/1586015
vec3 uniform_on_sphere(inout RandomState state)
{
    float phi = 2 * PI * random(state);
    float cos_theta = 2 * random(state) - 1;
    float sin_theta = sqrt(1 - cos_theta * cos_theta);

    return vec3(sin_theta*cos(phi), cos_theta, sin_theta*sin(phi));
}

vec3 cosine_weighted_on_hemisphere(inout RandomState state, in vec3 normal)
{
    return normalize(normal + uniform_on_sphere(state));
}

#endif
