import open3d as o3d
import numpy as np
import sys


def preprocess(pcd, voxel_size):
    pcd_down = pcd.voxel_down_sample(voxel_size)
    pcd_down.estimate_normals(
        o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size * 2, max_nn=30))
    fpfh = o3d.pipelines.registration.compute_fpfh_feature(
        pcd_down,
        o3d.geometry.KDTreeSearchParamHybrid(radius=voxel_size * 5, max_nn=100))
    return pcd_down, fpfh


def align_pair(source, target, voxel_size):
    s_down, s_fpfh = preprocess(source, voxel_size)
    t_down, t_fpfh = preprocess(target, voxel_size)
    result = o3d.pipelines.registration.registration_fgr_based_on_feature_matching(
        s_down, t_down, s_fpfh, t_fpfh,
        o3d.pipelines.registration.FastGlobalRegistrationOption(
            maximum_correspondence_distance=voxel_size * 5))
    result = o3d.pipelines.registration.registration_icp(
        source, target, voxel_size * 1.5, result.transformation,
        o3d.pipelines.registration.TransformationEstimationPointToPlane())
    return result.transformation


if __name__ == "__main__":
    if len(sys.argv) < 3:
        print("Usage: fgr_multi_align.py voxel cloud1.ply cloud2.ply [cloud3.ply ...]")
        sys.exit(1)

    try:
        voxel = float(sys.argv[1])
        paths = sys.argv[2:]
    except ValueError:
        voxel = 0.05
        paths = sys.argv[1:]

    clouds = [o3d.io.read_point_cloud(p) for p in paths]
    base = clouds[0]
    transforms = [np.eye(4)]
    for cloud in clouds[1:]:
        T = align_pair(cloud, base, voxel)
        cloud.transform(T)
        transforms.append(T)
        base += cloud

    for T in transforms:
        flat = " ".join(f"{v:.6f}" for v in T.flatten())
        print(flat)
