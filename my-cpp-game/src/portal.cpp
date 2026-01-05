#include "portal.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

/**
 * @brief Portalクラスのメソッドバインド
 * 
 */
void Portal::_bind_methods()
{
    // 1. シグナル用の関数を登録
    ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &Portal::_on_body_entered);

    // 2. エディタで設定できるプロパティを登録
    ClassDB::bind_method(D_METHOD("set_next_scene_path", "path"), &Portal::set_next_scene_path);
    ClassDB::bind_method(D_METHOD("get_next_scene_path"), &Portal::get_next_scene_path);

    // プロパティとして公開（ファイル選択ウィンドウが出るようにヒントを付ける）
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "next_scene_path", PROPERTY_HINT_FILE, "*.tscn"), "set_next_scene_path", "get_next_scene_path");
}

Portal::Portal()
{
    next_scene_path = "";
}

Portal::~Portal()
{
}

void Portal::_ready()
{
    // 自身の body_entered シグナルを、自分の _on_body_entered 関数に接続
    connect("body_entered", Callable(this, "_on_body_entered"));
}

void Portal::set_next_scene_path(const String &p_path)
{
    next_scene_path = p_path;
}

String Portal::get_next_scene_path() const
{
    return next_scene_path;
}

void Portal::_on_body_entered(Node* body)
{
    // 触れたのがプレイヤーかチェック（グループ名 "player" を確認）
    // ※ player.tscn のグループ設定が "player" (小文字) である前提です
    if (body->is_in_group("player"))
    {
        if (next_scene_path.is_empty())
        {
            UtilityFunctions::print("Error: Next scene path is empty!");
            return;
        }

        // シーン移動を実行！
        UtilityFunctions::print("Traveling to: ", next_scene_path);
        
        // call_deferredを使うのが安全（物理判定中のシーン変更によるクラッシュを防ぐため）
        get_tree()->call_deferred("change_scene_to_file", next_scene_path);
    }
}