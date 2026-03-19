#include "block_creature_mesh.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>
#include <godot_cpp/variant/utility_functions.hpp> // print関数を使うため

using namespace godot;

void BlockCreatureMesh::_bind_methods() {
    // GDScriptから呼べるように関数を登録
    ClassDB::bind_method(D_METHOD("setup"), &BlockCreatureMesh::setup);
    ClassDB::bind_method(D_METHOD("add_block", "pos", "type"), &BlockCreatureMesh::add_block, DEFVAL("normal"));
    ClassDB::bind_method(D_METHOD("get_mass"), &BlockCreatureMesh::get_mass);
    ClassDB::bind_method(D_METHOD("get_speed"), &BlockCreatureMesh::get_speed);
}

BlockCreatureMesh::BlockCreatureMesh() {
    current_mass = 1.0f;
    current_speed = 5.0f;
}

BlockCreatureMesh::~BlockCreatureMesh() {}

// 初期化処理（一番弱い1ブロックだけにする）
void BlockCreatureMesh::setup() {
    blocks.clear();
    blocks[Vector3(0, 0, 0)] = "normal"; // コアブロック
    generate_mesh();
    update_stats();
}

// ブロックの追加
void BlockCreatureMesh::add_block(const Vector3& pos, const String& type) {
    if (!blocks.has(pos)) {
        blocks[pos] = type;
        generate_mesh();
        update_stats();
    }
}

// ステータス計算（動物番長の法則！）
void BlockCreatureMesh::update_stats() {
    current_mass = (float)blocks.size();

    float min_x = 0.0f, max_x = 0.0f;
    float min_z = 0.0f, max_z = 0.0f;
    int red_count = 0;

    Array keys = blocks.keys();
    for (int i = 0; i < keys.size(); i++) {
        Vector3 pos = keys[i];
        String type = blocks[pos];

        if (pos.x < min_x) min_x = pos.x;
        if (pos.x > max_x) max_x = pos.x;
        if (pos.z < min_z) min_z = pos.z;
        if (pos.z > max_z) max_z = pos.z;

        if (type == "red") {
            red_count++;
        }
    }

    float width = (max_x - min_x) + 1.0f;
    float length = (max_z - min_z) + 1.0f;

    // 例：縦長だと速い(+)、横広だと空気抵抗で遅い(-)、赤ブロックが多いと筋力アップ(+)
    current_speed = 5.0f + (length * 1.5f) - (width * 0.5f) + (red_count * 2.0f);
    current_speed -= (current_mass * 0.2f); // 重すぎると遅くなるペナルティ

    if (current_speed < 1.0f) {
        current_speed = 1.0f;
    }

    // C++側からGodotのエディタにプリント出力
    UtilityFunctions::print("[C++] 進化！ 重さ: ", current_mass, " / 速度: ", current_speed, " / 形状(幅x奥): ", width, "x", length);
}

// メッシュ生成（引数なしで、自分自身のblocksを元に作るように変更）
void BlockCreatureMesh::generate_mesh() {
    Ref<SurfaceTool> st;
    st.instantiate();
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    Array keys = blocks.keys();
    for (int i = 0; i < keys.size(); i++) {
        Vector3 pos = keys[i];
        float s = 0.5f;

        Vector3 p0 = pos + Vector3(-s, -s, -s);
        Vector3 p1 = pos + Vector3( s, -s, -s);
        Vector3 p2 = pos + Vector3( s,  s, -s);
        Vector3 p3 = pos + Vector3(-s,  s, -s);
        Vector3 p4 = pos + Vector3(-s, -s,  s);
        Vector3 p5 = pos + Vector3( s, -s,  s);
        Vector3 p6 = pos + Vector3( s,  s,  s);
        Vector3 p7 = pos + Vector3(-s,  s,  s);

        // 前面(Front)
        st->add_vertex(p4); st->add_vertex(p5); st->add_vertex(p6);
        st->add_vertex(p4); st->add_vertex(p6); st->add_vertex(p7);
        // 後面(Back)
        st->add_vertex(p1); st->add_vertex(p0); st->add_vertex(p3);
        st->add_vertex(p1); st->add_vertex(p3); st->add_vertex(p2);
        // 左面(Left)
        st->add_vertex(p0); st->add_vertex(p4); st->add_vertex(p7);
        st->add_vertex(p0); st->add_vertex(p7); st->add_vertex(p3);
        // 右面(Right)
        st->add_vertex(p5); st->add_vertex(p1); st->add_vertex(p2);
        st->add_vertex(p5); st->add_vertex(p2); st->add_vertex(p6);
        // 上面(Top)
        st->add_vertex(p7); st->add_vertex(p6); st->add_vertex(p2);
        st->add_vertex(p7); st->add_vertex(p2); st->add_vertex(p3);
        // 下面(Bottom)
        st->add_vertex(p0); st->add_vertex(p1); st->add_vertex(p5);
        st->add_vertex(p0); st->add_vertex(p5); st->add_vertex(p4);
    }

    st->generate_normals();
    Ref<ArrayMesh> mesh = st->commit();
    set_mesh(mesh);
}

float BlockCreatureMesh::get_mass() const { return current_mass; }
float BlockCreatureMesh::get_speed() const { return current_speed; }