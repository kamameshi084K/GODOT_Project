#include "warp_area.hpp"
#include "game_manager.hpp" // GameManagerを使うために必要
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void WarpArea::_bind_methods()
{
    // 1. シグナル用の関数を登録
    ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &WarpArea::_on_body_entered);

    // 2. エディタ設定プロパティ（シーンパス）
    ClassDB::bind_method(D_METHOD("set_target_scene_path", "path"), &WarpArea::set_target_scene_path);
    ClassDB::bind_method(D_METHOD("get_target_scene_path"), &WarpArea::get_target_scene_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "target_scene_path", PROPERTY_HINT_FILE, "*.tscn"), "set_target_scene_path", "get_target_scene_path");

    // 3. エディタ設定プロパティ（出現座標）
    ClassDB::bind_method(D_METHOD("set_target_position", "pos"), &WarpArea::set_target_position);
    ClassDB::bind_method(D_METHOD("get_target_position"), &WarpArea::get_target_position);
    ADD_PROPERTY(PropertyInfo(Variant::VECTOR3, "target_position"), "set_target_position", "get_target_position");
}

WarpArea::WarpArea()
{
    target_scene_path = "";
    target_position = Vector3(0, 0, 0);
}

WarpArea::~WarpArea()
{
}

void WarpArea::_ready()
{

}

void WarpArea::_on_body_entered(Node* body)
{
    // プレイヤーが触れた時だけ実行
    if (body->is_in_group("player"))
    {
        if (target_scene_path.is_empty())
        {
            UtilityFunctions::print("Error: Target scene path is empty!");
            return;
        }

        UtilityFunctions::print("Warping to: ", target_scene_path);

        // GameManagerに行き先の座標を預ける
        GameManager* gm = GameManager::get_singleton();
        if (gm)
        {
            gm->set_last_player_position(target_position);
            
            // Playerの_readyで位置を復元させるためにフラグを立てる
            // (名前はreturning_from_battleですが、ワープ復帰としても流用します)
            gm->set_is_returning_from_battle(true);
        }

        // 安全にシーンを変更
        get_tree()->call_deferred("change_scene_to_file", target_scene_path);
    }
}

// --- セッター・ゲッター実装 ---

void WarpArea::set_target_scene_path(const String &p_path)
{
    target_scene_path = p_path;
}

String WarpArea::get_target_scene_path() const
{
    return target_scene_path;
}

void WarpArea::set_target_position(const Vector3 &p_pos)
{
    target_position = p_pos;
}

Vector3 WarpArea::get_target_position() const
{
    return target_position;
}