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
    ClassDB::bind_method(D_METHOD("get_unique_vertices", "hex_radius"), &Board::get_unique_vertices);
    ClassDB::bind_method(D_METHOD("get_unique_edges", "hex_radius"), &Board::get_unique_edges);
}

Array Board::get_unique_vertices(float hex_radius)
{
    Array unique_points;
    // 重複とみなす距離（SFMLの mergeDistance と同じ考え方です）
    float merge_distance = hex_radius * 0.4f; 
    
    // 子ノード（HexTile）をすべて取得
    TypedArray<Node> children = get_children();
    
    for (int i = 0; i < children.size(); i++) {
        HexTile* tile = Object::cast_to<HexTile>(children[i]);
        if (tile) {
            Vector2 center = tile->get_position();
            
            // SFML版と同じく、6つの角の座標を計算
            for (int j = 0; j < 6; ++j) {
                // 上が平らな六角形（Flat-top）の角度計算
                float angle_deg = 60.0f * j - 30.0f;
                float angle_rad = Math_PI / 180.0f * angle_deg;
                
                // 頂点のグローバル（Board基準）座標
                Vector2 point = center + Vector2(cos(angle_rad), sin(angle_rad)) * hex_radius;
                
                // すでに登録済みの頂点と近すぎないか（重複チェック）
                bool is_duplicate = false;
                for (int k = 0; k < unique_points.size(); k++) {
                    Vector2 existing_point = unique_points[k];
                    if (point.distance_to(existing_point) < merge_distance) {
                        is_duplicate = true;
                        break;
                    }
                }
                
                // 重複していなければリストに追加
                if (!is_duplicate) {
                    unique_points.push_back(point);
                }
            }
        }
    }
    return unique_points;
}

Array Board::get_unique_edges(float hex_radius)
{
    Array unique_edges;
    // 重複とみなす距離（中心点同士の距離で判定）
    float merge_distance = hex_radius * 0.4f; 
    
    TypedArray<Node> children = get_children();
    
    for (int i = 0; i < children.size(); i++) {
        HexTile* tile = Object::cast_to<HexTile>(children[i]);
        if (tile) {
            Vector2 center = tile->get_position();
            Vector2 vertices[6];
            
            // 6つの頂点を計算
            for (int j = 0; j < 6; ++j) {
                float angle_rad = Math_PI / 180.0f * (60.0f * j - 30.0f);
                vertices[j] = center + Vector2(cos(angle_rad), sin(angle_rad)) * hex_radius;
            }
            
            // 6つの辺を計算
            for (int j = 0; j < 6; ++j) {
                Vector2 p1 = vertices[j];
                Vector2 p2 = vertices[(j + 1) % 6];
                Vector2 midpoint = (p1 + p2) / 2.0f; // 辺の中心点
                
                // すでに登録済みの辺と重複していないか（中心点同士の距離でチェック）
                bool is_duplicate = false;
                for (int k = 0; k < unique_edges.size(); k++) {
                    Dictionary existing_edge = unique_edges[k];
                    Vector2 existing_midpoint = existing_edge["midpoint"];
                    if (midpoint.distance_to(existing_midpoint) < merge_distance) {
                        is_duplicate = true;
                        break;
                    }
                }
                
                // 重複していなければリストに追加
                if (!is_duplicate) {
                    Dictionary edge_data;
                    edge_data["start"] = p1;
                    edge_data["end"] = p2;
                    edge_data["midpoint"] = midpoint;
                    unique_edges.push_back(edge_data);
                }
            }
        }
    }
    return unique_edges;
}