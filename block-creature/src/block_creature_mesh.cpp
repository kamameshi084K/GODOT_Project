#include "block_creature_mesh.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/surface_tool.hpp>
#include <godot_cpp/classes/array_mesh.hpp>

using namespace godot;

void BlockCreatureMesh::_bind_methods() {
    // GDScriptから呼べるように関数を登録
    ClassDB::bind_method(D_METHOD("generate_mesh", "block_positions"), &BlockCreatureMesh::generate_mesh);
}

BlockCreatureMesh::BlockCreatureMesh() {}
BlockCreatureMesh::~BlockCreatureMesh() {}

void BlockCreatureMesh::generate_mesh(const Array& block_positions) {
    Ref<SurfaceTool> st;
    st.instantiate();

    // 三角形ポリゴンを作るモードで開始
    st->begin(Mesh::PRIMITIVE_TRIANGLES);

    // 各ブロックの座標に対して、四角形（キューブ）を作る
    for (int i = 0; i < block_positions.size(); i++) {
        Vector3 pos = block_positions[i];

        // キューブのサイズ（中心からの距離＝0.5にすると1x1x1のブロックになる）
        float s = 0.5f;

        // キューブを構成する8つの頂点
        Vector3 p0 = pos + Vector3(-s, -s, -s);
        Vector3 p1 = pos + Vector3( s, -s, -s);
        Vector3 p2 = pos + Vector3( s,  s, -s);
        Vector3 p3 = pos + Vector3(-s,  s, -s);
        Vector3 p4 = pos + Vector3(-s, -s,  s);
        Vector3 p5 = pos + Vector3( s, -s,  s);
        Vector3 p6 = pos + Vector3( s,  s,  s);
        Vector3 p7 = pos + Vector3(-s,  s,  s);

        // 6つの面（それぞれ2つの三角形）を追加していく
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

    // 光（陰影）が正しく当たるように、面の向き（法線）を自動計算する
    st->generate_normals();

    // メッシュを完成させて、自分自身（MeshInstance3D）にセットする
    Ref<ArrayMesh> mesh = st->commit();
    set_mesh(mesh);
}