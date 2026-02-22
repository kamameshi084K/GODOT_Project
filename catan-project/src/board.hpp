#pragma once

#include <godot_cpp/classes/node2d.hpp>
#include <godot_cpp/templates/hash_map.hpp> // Godotの連想配列
#include "hex_tile.hpp" // HexTileを知っている必要がある

namespace godot {

class Board : public Node2D {
    GDCLASS(Board, Node2D)

private:
    // 座標 (Vector2i) をキーにして、HexTileへのポインタを保存する辞書
    // 例: (0, 0) -> HexTile*
    HashMap<Vector2i, HexTile*> tile_map;

protected:
    static void _bind_methods();

public:
    Board();
    ~Board();

    void _ready() override;

    // 座標を指定してタイルを取得する便利関数
    // C++やGDScriptから「board.get_tile(0, 0)」のように呼べるようになります
    HexTile* get_tile(int q, int r);

    Array get_unique_vertices(float hex_radius);

    Array get_unique_edges(float hex_radius);
};

} // namespace godot