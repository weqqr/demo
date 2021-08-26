#pragma once

#include <Demo/Voxels/Voxel.h>

#include <vector>

namespace Demo {
class DenseGrid {
public:
    explicit DenseGrid(VoxelSource& source);
    const std::vector<uint32_t>& data() const { return m_data; }

private:
    std::vector<uint32_t> m_data = {};
};
}
