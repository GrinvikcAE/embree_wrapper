#include <pybind11/pybind11.h>
#include <pybind11/numpy.h>
#include <embree4/rtcore.h>
#include <embree4/rtcore_ray.h>
#include <limits>
#include <iostream>

namespace py = pybind11;

struct GeometryNormal {
    float x;
    float y;
    float z;
};

struct HitResult {
    bool hit;
    float u;
    float v;
    float t;
    unsigned geomID;
    py::array_t<float> position;
    GeometryNormal normal;
};

class EmbreeDevice {
public:
    EmbreeDevice() {
        device = rtcNewDevice(nullptr);
        if (!device) {
            throw std::runtime_error("Failed to create Embree device.");
        }
    }

    EmbreeDevice(const EmbreeDevice&) = delete;
    EmbreeDevice& operator=(const EmbreeDevice&) = delete;

    ~EmbreeDevice() {
        if (device) {
            rtcReleaseDevice(device);
        }
    }

    void* get() const { return device; }

private:
    RTCDevice device;
};

class EmbreeScene {
public:
    EmbreeScene(void* device_ptr) {
        device = static_cast<RTCDevice>(device_ptr);
        scene = rtcNewScene(device);
    }

    ~EmbreeScene() {
        rtcReleaseScene(scene);
    }

    void add_triangle_mesh(py::array_t<float> vertices, py::array_t<unsigned> indices) {
        auto verts = vertices.unchecked<2>();
        auto tris = indices.unchecked<2>();

        if (verts.shape(1) != 3) {
            throw std::runtime_error("Vertices must have shape [N,3].");
        }
        if (tris.shape(1) != 3) {
            throw std::runtime_error("Indices must have shape [M,3].");
        }

        RTCGeometry geom = rtcNewGeometry(device, RTC_GEOMETRY_TYPE_TRIANGLE);

        float* vb = (float*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_VERTEX, 0,
                                                 RTC_FORMAT_FLOAT3, 3*sizeof(float), verts.shape(0));

        unsigned* ib = (unsigned*)rtcSetNewGeometryBuffer(geom, RTC_BUFFER_TYPE_INDEX, 0,
                                                        RTC_FORMAT_UINT3, 3*sizeof(unsigned), tris.shape(0));

        for (py::ssize_t i = 0; i < verts.shape(0); ++i) {
            vb[3*i+0] = verts(i, 0);
            vb[3*i+1] = verts(i, 1);
            vb[3*i+2] = verts(i, 2);
        }

        for (py::ssize_t i = 0; i < tris.shape(0); ++i) {
            unsigned v0 = tris(i, 0);
            unsigned v1 = tris(i, 1);
            unsigned v2 = tris(i, 2);

            ib[3*i+0] = v0;
            ib[3*i+1] = v1;
            ib[3*i+2] = v2;
        }

        rtcCommitGeometry(geom);
        rtcAttachGeometry(scene, geom);
        rtcReleaseGeometry(geom);
    }

    void commit() {
        rtcCommitScene(scene);
    }

    HitResult intersect_ray(py::array_t<float> origin, py::array_t<float> direction) {
        auto org = origin.unchecked<1>();
        auto dir = direction.unchecked<1>();

        float dir_length = sqrt(dir(0)*dir(0) + dir(1)*dir(1) + dir(2)*dir(2));
        float inv_dir_length = 1.0f / dir_length;
        float normalized_dir[3] = {
            dir(0) * inv_dir_length,
            dir(1) * inv_dir_length,
            dir(2) * inv_dir_length
        };
        RTCRayHit rayhit;
        memset(&rayhit, 0, sizeof(rayhit));

        rayhit.ray.org_x = org(0);
        rayhit.ray.org_y = org(1);
        rayhit.ray.org_z = org(2);
        rayhit.ray.dir_x = normalized_dir[0];
        rayhit.ray.dir_y = normalized_dir[1];
        rayhit.ray.dir_z = normalized_dir[2];
        rayhit.ray.tnear = 0.001f;
        rayhit.ray.tfar = std::numeric_limits<float>::infinity();
        rayhit.ray.mask = -1;
        rayhit.ray.flags = 0;
//        rayhit.hit.geomID = RTC_INVALID_GEOMETRY_ID;
        rayhit.hit.geomID = -1;

        rtcIntersect1(scene, &rayhit);

        HitResult result;
//        result.hit = (rayhit.hit.geomID != RTC_INVALID_GEOMETRY_ID);
        result.hit = (rayhit.hit.geomID != -1);

        if (result.hit) {
            result.t = rayhit.ray.tfar;
            result.u = rayhit.hit.u;
            result.v = rayhit.hit.v;
            result.geomID = rayhit.hit.geomID;
            result.normal.x = rayhit.hit.Ng_x;
            result.normal.y = rayhit.hit.Ng_y;
            result.normal.z = rayhit.hit.Ng_z;

            // Вычисление позиции пересечения
            result.position = py::array_t<float>(3);
            auto pos = result.position.mutable_unchecked<1>();
            pos(0) = org(0) + rayhit.ray.tfar * normalized_dir[0];
            pos(1) = org(1) + rayhit.ray.tfar * normalized_dir[1];
            pos(2) = org(2) + rayhit.ray.tfar * normalized_dir[2];
        } else {
            result.t = rayhit.ray.tfar;
            result.u = rayhit.hit.u;
            result.v = rayhit.hit.v;
            result.geomID = rayhit.hit.geomID;
        }

        return result;
    }

private:
    RTCScene scene;
    RTCDevice device;
};

PYBIND11_MODULE(embree_wrapper, module) {

    py::class_<GeometryNormal>(module, "GeometryNormal")
        .def_readonly("x", &GeometryNormal::x)
        .def_readonly("y", &GeometryNormal::y)
        .def_readonly("z", &GeometryNormal::z);

    py::class_<HitResult>(module, "HitResult")
        .def_readonly("hit", &HitResult::hit)
        .def_readonly("u", &HitResult::u)
        .def_readonly("v", &HitResult::v)
        .def_readonly("t", &HitResult::t)
        .def_readonly("geomID", &HitResult::geomID)
        .def_readonly("position", &HitResult::position)
        .def_readonly("normal", &HitResult::normal);

    py::class_<EmbreeDevice>(module, "Device")
        .def(py::init<>())
        .def("get", &EmbreeDevice::get);

    py::class_<EmbreeScene>(module, "Scene")
        .def(py::init<void*>())
        .def("add_triangle_mesh", &EmbreeScene::add_triangle_mesh)
        .def("commit", &EmbreeScene::commit)
        .def("intersect_ray", &EmbreeScene::intersect_ray);
}