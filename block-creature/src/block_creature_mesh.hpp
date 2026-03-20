#ifndef BLOCK_CREATURE_MESH_HPP
#define BLOCK_CREATURE_MESH_HPP

#include <godot_cpp/classes/mesh_instance3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot {

class BlockCreatureMesh : public MeshInstance3D {
    GDCLASS(BlockCreatureMesh, MeshInstance3D)

private:
    // ブロックの座標と種類（色など）を記憶する辞書
    Dictionary blocks; 
    
    // ステータス
    float current_mass;
    float current_speed;

    // 内部で使う処理
    void generate_mesh();
    void update_stats();

protected:
    static void _bind_methods();

public:
    BlockCreatureMesh();
    ~BlockCreatureMesh();

<<<<<<< Updated upstream
    // GDScriptから呼べるようにする関数
    void setup(); 
    void add_block(const Vector3& pos, const String& type);
    float get_mass() const;
    float get_speed() const;
=======
    // ★ ここが心臓部：ブロックの座標リストを受け取ってメッシュを生成する関数
    void generate_mesh(const PackedVector3Array& positions, const PackedInt32Array& types);
>>>>>>> Stashed changes
};

}
#endif