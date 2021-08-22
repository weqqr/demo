#include <Demo/Mesh.h>

namespace Demo {


void Mesh::add_vertex(Vertex vertex)
{
    m_data.push_back(vertex.position.x);
    m_data.push_back(vertex.position.y);
    m_data.push_back(vertex.position.z);
    m_data.push_back(vertex.normal.x);
    m_data.push_back(vertex.normal.y);
    m_data.push_back(vertex.normal.z);
    m_data.push_back(vertex.uv.x);
    m_data.push_back(vertex.uv.y);
}
}
