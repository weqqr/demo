#include <Demo/Common/Base.h>
#include <Demo/Common/Log.h>
#include <Demo/Voxels/DenseGrid.h>

namespace Demo {
DenseGrid::DenseGrid(VoxelSource& source)
{
    auto size = source.size();

    ASSERT(size.x % 4 == 0 && size.y % 4 == 0 && size.z % 4 == 0);

    for (uint32_t y = 0; y < size.y; y++) {
        for (uint32_t z = 0; z < size.z; z++) {
            for (uint32_t x = 0; x < size.x; x += 4) {
                auto voxel0 = source.get_voxel({x, y, z}).id;
                auto voxel1 = source.get_voxel({x, y, z}).id;
                auto voxel2 = source.get_voxel({x, y, z}).id;
                auto voxel3 = source.get_voxel({x, y, z}).id;

                uint32_t compound = (voxel0 << 24) | (voxel1 << 16) | (voxel2 << 8) | voxel3;
                m_data.push_back(compound);
            }
        }
    }
}
}
