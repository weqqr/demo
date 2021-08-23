#version 450

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
} uniforms;

const float PI = 3.1415926;
const vec3  UP = vec3(0, 1, 0);
const vec3  SUN = normalize(vec3(0.3, 0.5, 0.7));

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

Ray generate_ray()
{
    vec3 origin = uniforms.position.xyz;
    vec3 look_dir = normalize(uniforms.look_dir.xyz);

    float scale = 2 * tan(degrees_to_radians(uniforms.fov) / 2);
    vec3 u = scale * normalize(cross(look_dir, UP)) * uniforms.aspect_ratio;
    vec3 v = scale * normalize(cross(u, look_dir));

    vec3 dir = normalize(look_dir + (v_uv.x - 0.5) * u + (v_uv.y - 0.5) * v);
    vec3 inv_dir = 1.0 / dir;

    return Ray(origin, dir, inv_dir);
}

void main()
{
    Ray ray = generate_ray();

    color = vec4(0, 0, 0, 1);
    vec3 center = vec3(0, 0, 0);
    const vec3 radius = vec3(0.5, 0.5, 0.5) + 0.1 * cos(uniforms.time * PI);
    const vec3 inv_radius = 1.0 / radius;

    Box box = {center, radius, inv_radius};

    float distance;
    vec3 normal;
    bool intersects = intersect(box, ray, distance, normal);

    if (intersects) {
        color = vec4(0.3 + vec3(0.7) * clamp(-dot(normal, SUN), 0, 1), 1);
    }
}
