#pragma once

#include <glm/glm.hpp>
#include "shape.h"
#include "utility/vec3.h"
#include <memory>

using namespace std;


class triangle : public shape {
public:
    triangle(const point3& A,
        const point3& B,
        const point3& C,
        std::shared_ptr<material> mat);

    point3 get_center() const override;
    vec3 random(const point3& origin) const override;
    bool hit(const ray& r, interval ray_t, hit_record& rec) const override;
	bool emits() const override;
    double pdf_value(const point3& origin, const vec3& direction) const override;

    
private:
    double D;
    vec3 u, v;
    vec3 normal;
    double area;
    point3 A, B, C;
    double uu, uv, vv, inv_denom;

    std::shared_ptr<material> mat;
};

