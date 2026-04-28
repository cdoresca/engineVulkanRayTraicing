#include "shape.h"


// ------------------------------------------------------------
// hit_record methods

void hit_record::set_face_normal(const ray& r, const vec3& outward_normal) {
	// outward_normal is assumed to be unit length
	front_face = dot(r.direction(), outward_normal) < 0;
	normal = front_face ? outward_normal : -outward_normal;
}