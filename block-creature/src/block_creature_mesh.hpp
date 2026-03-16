#pragma once

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot {

class BlockCreatureMesh : public MeshInstance3D {
    GDCLASS(BlockCreatureMesh, MeshInstance3D)

protected:
    static void _bind_methods();

public:
    BlockCreatureMesh();
    ~BlockCreatureMesh();

    // ★ ここが心臓部：ブロックの座標リストを受け取ってメッシュを生成する関数
    void generate_mesh(const Array& block_positions);
};

} // namespace godot