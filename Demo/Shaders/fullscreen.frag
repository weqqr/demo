#version 450

#extension GL_GOOGLE_include_directive : require

#include "common.glsl"
#include "random.glsl"

layout (location = 0) in vec2 v_uv;

layout (location = 0) out vec4 color;

layout (push_constant) uniform PushConstants {
    float time;
} push_constants;

layout (set = 0, binding = 0) uniform Uniforms {
    vec4 position;
    vec4 look_dir;
    float time;
    float aspect_ratio;
    float fov;
    float width;
    float height;
} uniforms;

const vec3 UP = vec3(0, 1, 0);
const vec3 SUN = normalize(vec3(0.3, 0.5, 0.7));
const uint MAX_BOUNCES = 4;
const uint SAMPLES = 4;

struct Ray {
    vec3 origin;
    vec3 dir;
    vec3 inv_dir;
};

struct Box {
    vec3 center;
    vec3 radius;
    vec3 inv_radius;
};

float maxf(in vec3 v)
{
    return max(max(v.x, v.y), v.z);
}

// Majercik, Journal of Computer Graphics Techniques (JCGT), vol. 7, no. 3
// http://jcgt.org/published/0007/03/04/
bool intersect(in Box box, in Ray ray, out float distance, out vec3 normal)
{
    ray.origin = ray.origin - box.center;
    float winding = (maxf(abs(ray.origin) * box.inv_radius) < 1.0) ? -1 : 1;
    vec3 sgn = -sign(ray.dir);
    vec3 d = box.radius * winding * sgn - ray.origin;
    d *= ray.inv_dir;
#define TEST(U, VW) (d.U >= 0.0) && all(lessThan(abs(ray.origin.VW + ray.dir.VW * d.U), box.radius.VW))
    bvec3 test = bvec3(TEST(x, yz), TEST(y, zx), TEST(z, xy));
    sgn = test.x ? vec3(sgn.x, 0, 0) : (test.y ? vec3(0, sgn.y, 0) : vec3(0, 0, test.z ? sgn.z : 0));
#undef TEST
    distance = (sgn.x != 0) ? d.x : ((sgn.y != 0) ? d.y : d.z);
    normal = sgn;
    return (sgn.x != 0) || (sgn.y != 0) || (sgn.z != 0);
}

float degrees_to_radians(in float degrees)
{
    return degrees / 180.0 * PI;
}

Ray generate_ray(in RandomState state)
{
    vec3 origin = uniforms.position.xyz;
    vec3 look_dir = normalize(uniforms.look_dir.xyz);

    float u_jitter = random(state) / uniforms.width;
    float v_jitter = random(state) / uniforms.height;

    float scale = 2 * tan(degrees_to_radians(uniforms.fov) / 2);
    vec3 u = scale * normalize(cross(look_dir, UP)) * uniforms.aspect_ratio;
    vec3 v = scale * normalize(cross(u, look_dir));

    vec3 dir = normalize(look_dir + (v_uv.x - 0.5 + u_jitter) * u + (v_uv.y - 0.5 + v_jitter) * v);
    vec3 inv_dir = 1.0 / dir;

    return Ray(origin, dir, inv_dir);
}

bool intersect_scene(in Ray ray, out float distance, out vec3 normal, out vec3 albedo)
{
    vec3 center = vec3(0, 0, 0);
    const vec3 radius = vec3(0.5, 0.5, 0.5) + 0.1 * cos(uniforms.time * PI);
    const vec3 inv_radius = 1.0 / radius;

    Box box = {center, radius, inv_radius};

    bool intersects = intersect(box, ray, distance, normal);

    albedo = vec3(0.9, 0.8, 0.7);

    return intersects;
}

vec3 raytrace_entire_thing(in Ray ray, inout RandomState state)
{
    vec3 color = vec3(0);
    vec3 throughput = vec3(1);

    float distance;
    vec3 normal;

    for (int i = 0; i < MAX_BOUNCES; i++) {
        vec3 albedo;
        bool intersects = intersect_scene(ray, distance, normal, albedo);

        if (!intersects) {
            vec3 sky = dot(ray.dir, -SUN) > 0.95 ? vec3(10, 10, 10) : vec3(0.7, 0.8, 0.9);
            color += throughput * sky;
            break;
        }

        ray.origin += distance * ray.dir + 0.00001 * normal;
        throughput *= albedo;

        ray.dir = cosine_weighted_on_hemisphere(state, normal);
        ray.inv_dir = 1 / ray.dir;
    }

    return color;
}

void main()
{
    RandomState state = RandomState(floatBitsToUint(v_uv.x * v_uv.y), floatBitsToUint(uniforms.time));

    color = vec4(0, 0, 0, 1);

    for (uint i = 0; i < SAMPLES; i++) {
        Ray ray = generate_ray(state);
        color += vec4(raytrace_entire_thing(ray, state), 1);
    }

    color /= SAMPLES;
}
