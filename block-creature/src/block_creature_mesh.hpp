#ifndef BLOCK_CREATURE_MESH_HPP
#define BLOCK_CREATURE_MESH_HPP

#include <godot_cpp/classes/mesh_instance3d.hpp>

namespace godot {

class BlockCreatureMesh : public MeshInstance3D {
    GDCLASS(BlockCreatureMesh, MeshInstance3D)

protected:
    static void _bind_methods();

public:
    BlockCreatureMesh();
    ~BlockCreatureMesh();

    // ★ ここが心臓部：ブロックの座標リストと種類の配列を受け取ってメッシュを生成する関数
    void generate_mesh(const PackedVector3Array& positions, const PackedInt32Array& types);

};

}
#endif