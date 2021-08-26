#pragma once

#include <Demo/Math.h>

namespace Demo {
struct Voxel {
    uint8_t id;

    constexpr explicit Voxel(uint8_t id)
        : id(id)
    {
    }

    static constexpr Voxel empty() { return Voxel(0); }
};

class VoxelSource {
public:
    virtual Vector3u size() const = 0;
    virtual Voxel get_voxel(Vector3u position) const = 0;
};

class ProceduralGrid : public VoxelSource {
public:
    explicit ProceduralGrid(Vector3u size)
        : m_size(size)
    {
    }

    Vector3u size() const override { return m_size; }
    Voxel get_voxel(Vector3u position) const override;

private:
    Vector3u m_size;
};
}
