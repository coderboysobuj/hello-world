#include <reactphysics3d/reactphysics3d.h>
#include <iostream>
int main() {
    rp3d::PhysicsCommon pc;
    rp3d::TriangleVertexArray* tva = new rp3d::TriangleVertexArray(0, nullptr, 0, 0, nullptr, 0, rp3d::TriangleVertexArray::VertexDataType::VERTEX_FLOAT_TYPE, rp3d::TriangleVertexArray::IndexDataType::INDEX_INTEGER_TYPE);
    rp3d::TriangleMesh* tm = pc.createTriangleMesh();
    tm->addSubpart(tva);
    rp3d::ConcaveMeshShape* cms = pc.createConcaveMeshShape(tm);
    std::cout << "success";
    return 0;
}
