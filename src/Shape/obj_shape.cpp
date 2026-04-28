#include "obj_shape.h"

#include <limits>

static RTCDevice g_device = nullptr;

RTCDevice get_device() {
    if (!g_device) g_device = rtcNewDevice(nullptr);
    return g_device;
}

static unsigned int upload_mesh(RTCScene scene, RTCDevice device,
    const std::vector<tri_data>& tris,
    unsigned int base_id)
{
    RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

    size_t n = tris.size();

    float* vbuf = (float*)rtcSetNewGeometryBuffer(
        geom, RTC_BUFFER_TYPE_VERTEX, 0,
        RTC_FORMAT_FLOAT3, 3 * sizeof(float), n * 3);

    int* ibuf = (int*)rtcSetNewGeometryBuffer(
        geom, RTC_BUFFER_TYPE_INDEX, 0,
        RTC_FORMAT_UINT3, 3 * sizeof(int), n);

    for (size_t i = 0; i < n; i++) {
        // vertices
        vbuf[i * 9 + 0] = tris[i].v0.x(); vbuf[i * 9 + 1] = tris[i].v0.y(); vbuf[i * 9 + 2] = tris[i].v0.z();
        vbuf[i * 9 + 3] = tris[i].v1.x(); vbuf[i * 9 + 4] = tris[i].v1.y(); vbuf[i * 9 + 5] = tris[i].v1.z();
        vbuf[i * 9 + 6] = tris[i].v2.x(); vbuf[i * 9 + 7] = tris[i].v2.y(); vbuf[i * 9 + 8] = tris[i].v2.z();
        // indices
        ibuf[i * 3 + 0] = i * 3; ibuf[i * 3 + 1] = i * 3 + 1; ibuf[i * 3 + 2] = i * 3 + 2;
    }

    rtcCommitGeometry(geom);
    unsigned int id = rtcAttachGeometry(scene, geom);
    rtcReleaseGeometry(geom);
    return id;
}

void obj_shape::commit_scene() {
    rtcCommitScene(embree_scene);
}

obj_shape::obj_shape(
    const point3& center,
    const std::vector<mesh>& meshes,
    const std::unordered_map<std::string, std::shared_ptr<material>>& mat_map,
    std::shared_ptr<material> fallback,
    shape_list& lights)
    : center(center)
{
    device = get_device();
    embree_scene = rtcNewScene(device);

    for (const auto& m : meshes) {
        std::shared_ptr<material> mat = fallback;
        auto it = mat_map.find(m.mat_name);
        if (it != mat_map.end()) mat = it->second;

        auto& verts = m.vertices;
        auto& indices = m.indices;

        for (size_t i = 0; i + 2 < indices.size(); i += 3) {
            const vertex& vert0 = verts[indices[i]];
            const vertex& vert1 = verts[indices[i + 1]];
            const vertex& vert2 = verts[indices[i + 2]];

            vec3 p0 = center + vec3(vert0.pos.x, vert0.pos.y, vert0.pos.z);
            vec3 p1 = center + vec3(vert1.pos.x, vert1.pos.y, vert1.pos.z);
            vec3 p2 = center + vec3(vert2.pos.x, vert2.pos.y, vert2.pos.z);

            vec3 e1 = p1 - p0;
            vec3 e2 = p2 - p0;
            vec3 normal = unit_vector(cross(e1, e2));

            triangles.push_back({
                mat,
                p0, p1, p2,
                normal,
                vert0.uv, vert1.uv, vert2.uv
                });

            if (mat->is_emissive())
                lights.add(std::make_shared<triangle>(p0, p1, p2, mat));
        }
    }

    upload_mesh(embree_scene, device, triangles, 0);
    commit_scene();
}

obj_shape::~obj_shape() {
    rtcReleaseScene(embree_scene);
}

bool obj_shape::hit(const ray& r, interval ray_t, hit_record& rec) const {
    RTCRayHit rh;
    rh.ray.org_x = r.origin().x(); rh.ray.org_y = r.origin().y(); rh.ray.org_z = r.origin().z();
    rh.ray.dir_x = r.direction().x(); rh.ray.dir_y = r.direction().y(); rh.ray.dir_z = r.direction().z();
    rh.ray.tnear = (float)ray_t.min;
    rh.ray.tfar = (float)ray_t.max;
    rh.ray.mask = -1;
    rh.ray.flags = 0;
    rh.hit.geomID = RTC_INVALID_GEOMETRY_ID;

    RTCIntersectArguments args;
    rtcInitIntersectArguments(&args);
    rtcIntersect1(embree_scene, &rh, &args);

    if (rh.hit.geomID == RTC_INVALID_GEOMETRY_ID)
        return false;

    rec.t = rh.ray.tfar;
    rec.p = r.at(rec.t);

    const tri_data& tri = triangles[rh.hit.primID];
    rec.mat = tri.mat;
    rec.set_face_normal(r, tri.normal);

    float w1 = rh.hit.u;
    float w2 = rh.hit.v;
    float w0 = 1.0f - w1 - w2;

    glm::vec2 interpolated_uv = w0 * tri.uv0 + w1 * tri.uv1 + w2 * tri.uv2;
    rec.u = interpolated_uv.x;
    rec.v = interpolated_uv.y;

    return true;
}


double obj_shape::pdf_value(const point3& origin, const vec3& direction) const {
    if (triangles.empty()) return 0.0;
    double sum = 0.0;
    for (auto& t : triangles) {
        vec3 e1 = t.v1 - t.v0, e2 = t.v2 - t.v0;
        double area = 0.5 * cross(e1, e2).length();
        vec3 to = t.v0 - origin;
        double dist2 = to.length_squared();
        if (dist2 < 1e-8) continue;
        double cos_a = std::abs(dot(unit_vector(direction), unit_vector(cross(e1, e2))));
        if (cos_a < 1e-8) continue;
        sum += dist2 / (cos_a * area);
    }
    return sum / triangles.size();
}

point3 obj_shape::get_center() const { return center; }