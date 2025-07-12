import numpy as np
from embree_wrapper import Device, Scene, HitResult, GeometryNormal

vertices = np.array([
    [0.0, 0.0, 0.0],
    [2.0, 0.0, 0.0],
    [2.0, 2.0, 0.0],
    [0.0, 2.0, 0.0],
], dtype=np.dtype("<f4"))

triangles = np.array([
    [0, 1, 2],
    [0, 2, 3],
], dtype=np.dtype("<u4"))

origins = np.array([
    [1.0, 1.0, 1.0],
    [0.1, 0.1, 1.0],
    [0, 0, 1.0],
    [0.5, 0.5, 2.0],
], dtype=np.dtype("<f4"))

directions = np.array([
    [0, 0, -1],
    [0, 0, -1],
    [0, 0, -1],
    [0, 0, -1.0],
], dtype=np.dtype("<f4"))

device = Device()
scene = Scene(device.get())
scene.add_triangle_mesh(vertices, triangles)
scene.commit()

for orig, direction in zip(origins, directions):
    result = scene.intersect_ray(orig, direction)
    print(
        f"Hit: {result.hit}\n"
        f" intersect_coords={result.position}\n"
        f" geomID={result.geomID}\n"
        f" u={result.u}\n"
        f" v={result.v}\n"
        f" t={result.t}\n"
        f"Mesh normally: {result.normal.x, result.normal.y, result.normal.z}"
    )