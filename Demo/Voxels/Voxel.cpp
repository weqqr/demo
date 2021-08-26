#include <Demo/Voxels/Voxel.h>

namespace Demo {
Voxel ProceduralGrid::get_voxel(Vector3u position) const
{
    if ((position.x ^ position.y ^ position.z) == 0) {
        return Voxel(1);
    }

    return Voxel::empty();
}
}
