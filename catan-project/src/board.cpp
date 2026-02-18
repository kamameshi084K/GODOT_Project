#include "board.hpp"
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

Board::Board() {
}

Board::~Board() {
}

void Board::_ready() {
    // 自分（Boardノード）の子ノードをすべて取得
    TypedArray<Node> children = get_children();

    // 子ノードを1つずつチェック
    for (int i = 0; i < children.size(); i++) {
        // HexTileクラスにキャスト（変換）してみる
        HexTile* tile = Object::cast_to<HexTile>(children[i]);

        // もしHexTileだったら辞書に登録
        if (tile) {
            Vector2i coord(tile->get_q(), tile->get_r());
            tile_map[coord] = tile;
            
            // デバッグ表示（確認用）
            UtilityFunctions::print("Tile registered at: ", coord);
        }
    }
}

HexTile* Board::get_tile(int q, int r) {
    Vector2i coord(q, r);
    
    // 辞書にその座標があるか確認
    if (tile_map.has(coord)) {
        return tile_map[coord];
    }
    
    // なければnullptrを返す
    return nullptr;
}

void Board::_bind_methods() {
    // get_tile関数をGDScriptからも呼べるように登録
    ClassDB::bind_method(D_METHOD("get_tile", "q", "r"), &Board::get_tile);
}