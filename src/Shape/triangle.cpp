#include "triangle.h"
#include "Material/material.h"

#include <cmath>

triangle::triangle(const point3& A,
    const point3& B,
    const point3& C,
    std::shared_ptr<material> mat)
    : A(A), B(B), C(C), mat(mat)
{
    u = B - A;
    v = C - A;

    auto n = cross(u, v);
    normal = unit_vector(n);
    D = dot(normal, A);
    area = 0.5 * n.length();

    uu = dot(u, u);
    uv = dot(u, v);
    vv = dot(v, v);
    inv_denom = 1.0 / (uv * uv - uu * vv);

    const double eps = 1e-4;

    point3 min(
        std::min({ A.x(), B.x(), C.x() }) - eps,
        std::min({ A.y(), B.y(), C.y() }) - eps,
        std::min({ A.z(), B.z(), C.z() }) - eps
    );

    point3 max(
        std::max({ A.x(), B.x(), C.x() }) + eps,
        std::max({ A.y(), B.y(), C.y() }) + eps,
        std::max({ A.z(), B.z(), C.z() }) + eps
    );
}

bool triangle::emits() const {
    return mat && mat->is_emissive();
}

bool triangle::hit(const ray& r, interval ray_t, hit_record& rec) const {
    const double EPS = 1e-8;

    vec3 h = cross(r.direction(), v);
    double a = dot(u, h);

    if (std::fabs(a) < EPS)
        return false;

    double f = 1.0 / a;
    vec3 s = r.origin() - A;
    double s_param = f * dot(s, h);

    if (s_param < 0.0 || s_param > 1.0)
        return false;

    vec3 q = cross(s, u);
    double t_param = f * dot(r.direction(), q);

    if (t_param < 0.0 || s_param + t_param > 1.0)
        return false;

    double t = f * dot(v, q);
    if (!ray_t.contains(t))
        return false;

    rec.t = t;
    rec.p = r.at(t);
    rec.mat = mat;
    rec.set_face_normal(r, normal);

    rec.u = s_param;
    rec.v = t_param;

    return true;
}


double triangle::pdf_value(const point3& origin, const vec3& direction) const {
    hit_record rec;
    if (!this->hit(ray(origin, direction), interval(0.001, infinity), rec))
        return 0;

    auto distance_squared = rec.t * rec.t * direction.length_squared();
    auto cosine = std::fabs(dot(direction, rec.normal) / direction.length());

    return distance_squared / (cosine * area);
}

vec3 triangle::random(const point3& origin) const {
    double r1 = random_double();
    double r2 = random_double();

    if (r1 + r2 > 1) {
        r1 = 1 - r1;
        r2 = 1 - r2;
    }

    point3 p = A + r1 * u + r2 * v;
    return p - origin;
}

point3 triangle::get_center() const {
    return (A + B + C) / 3.0;
}

