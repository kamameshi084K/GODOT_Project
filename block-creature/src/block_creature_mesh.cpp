#include "block_creature_mesh.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void BlockCreatureMesh::_bind_methods() {
    // 引数が2つになったので登録名を変更します
    ClassDB::bind_method(D_METHOD("generate_mesh", "positions", "types"), &BlockCreatureMesh::generate_mesh);
}

BlockCreatureMesh::BlockCreatureMesh() {}
BlockCreatureMesh::~BlockCreatureMesh() {}

void BlockCreatureMesh::generate_mesh(const PackedVector3Array& positions, const PackedInt32Array& types) {
    Ref<SurfaceTool> st;
    st.instantiate();

    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    for (int i = 0; i < positions.size(); i++) {
        Vector3 pos = positions[i];
        
        // 配列の安全確認（座標と種類の数が合っていない場合のエラー回避）
        int type = 0;
        if (i < types.size()) {
            type = types[i];
        }

        // ★追加：パーツの種類（ID）によって色を変える
        Color block_color;
        switch (type) {
            case 0: block_color = Color(0.5, 0.5, 0.5); break; // 0: 基本装甲（グレー）
            case 1: block_color = Color(1.0, 0.2, 0.2); break; // 1: スラスター/武器など（赤）
            case 2: block_color = Color(0.2, 0.5, 1.0); break; // 2: コアパーツ（青）
            default: block_color = Color(1.0, 1.0, 1.0); break; // その他（白）
        }
        
        // SurfaceToolに色を設定（これ以降に追加される頂点はこの色になります）
        st->set_color(block_color);

        float s = 0.5f;

        Vector3 p0 = pos + Vector3(-s, -s, -s);
        Vector3 p1 = pos + Vector3( s, -s, -s);
        Vector3 p2 = pos + Vector3( s,  s, -s);
        Vector3 p3 = pos + Vector3(-s,  s, -s);
        Vector3 p4 = pos + Vector3(-s, -s,  s);
        Vector3 p5 = pos + Vector3( s, -s,  s);
        Vector3 p6 = pos + Vector3( s,  s,  s);
        Vector3 p7 = pos + Vector3(-s,  s,  s);

        // ※面を追加する処理（st->add_vertex(...)）は今までと全く同じでOKです！
        // 前面
        st->add_vertex(p4); st->add_vertex(p5); st->add_vertex(p6);
        st->add_vertex(p4); st->add_vertex(p6); st->add_vertex(p7);
        // 後面
        st->add_vertex(p1); st->add_vertex(p0); st->add_vertex(p3);
        st->add_vertex(p1); st->add_vertex(p3); st->add_vertex(p2);
        // 左面
        st->add_vertex(p0); st->add_vertex(p4); st->add_vertex(p7);
        st->add_vertex(p0); st->add_vertex(p7); st->add_vertex(p3);
        // 右面
        st->add_vertex(p5); st->add_vertex(p1); st->add_vertex(p2);
        st->add_vertex(p5); st->add_vertex(p2); st->add_vertex(p6);
        // 上面
        st->add_vertex(p7); st->add_vertex(p6); st->add_vertex(p2);
        st->add_vertex(p7); st->add_vertex(p2); st->add_vertex(p3);
        // 下面
        st->add_vertex(p0); st->add_vertex(p1); st->add_vertex(p5);
        st->add_vertex(p0); st->add_vertex(p5); st->add_vertex(p4);
    }

    st->generate_normals();
    Ref<ArrayMesh> mesh = st->commit();
    set_mesh(mesh);
}