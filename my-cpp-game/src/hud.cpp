#include "hud.hpp"
#include "game_manager.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void HUD::_bind_methods()
{
    // HP Bar Path
    ClassDB::bind_method(D_METHOD("set_hp_bar_path", "path"), &HUD::set_hp_bar_path);
    ClassDB::bind_method(D_METHOD("get_hp_bar_path"), &HUD::get_hp_bar_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "hp_bar_path"), "set_hp_bar_path", "get_hp_bar_path");

    // Level Label Path
    ClassDB::bind_method(D_METHOD("set_level_label_path", "path"), &HUD::set_level_label_path);
    ClassDB::bind_method(D_METHOD("get_level_label_path"), &HUD::get_level_label_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "level_label_path"), "set_level_label_path", "get_level_label_path");

    // Exp Label Path
    ClassDB::bind_method(D_METHOD("set_exp_label_path", "path"), &HUD::set_exp_label_path);
    ClassDB::bind_method(D_METHOD("get_exp_label_path"), &HUD::get_exp_label_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "exp_label_path"), "set_exp_label_path", "get_exp_label_path");
}

HUD::HUD()
{
    hp_bar = nullptr;
    level_label = nullptr;
    exp_label = nullptr;
}

HUD::~HUD()
{
}

void HUD::_ready()
{
    // パスが設定されていればノードを取得する
    if (!hp_bar_path.is_empty())
    {
        hp_bar = get_node<ProgressBar>(hp_bar_path);
    }
    
    if (!level_label_path.is_empty())
    {
        level_label = get_node<Label>(level_label_path);
    }

    if (!exp_label_path.is_empty())
    {
        exp_label = get_node<Label>(exp_label_path);
    }
}

void HUD::_process(double delta)
{
    // GameManagerから情報を取得して表示を更新する
    GameManager* gm = GameManager::get_singleton();
    if (!gm)
    {
        return;
    }

    // HPバーの更新
    if (hp_bar)
    {
        hp_bar->set_max(gm->get_player_max_hp());
        hp_bar->set_value(gm->get_player_current_hp());
    }

    // レベル表示の更新
    if (level_label)
    {
        // "Lv. 5" のように表示
        level_label->set_text("Lv. " + String::num_int64(gm->get_player_level()));
    }

    // 経験値表示の更新
    if (exp_label)
    {
        // "Exp: 150 / 200" のように表示
        String exp_text = "Exp: " + String::num_int64(gm->get_player_exp()) + " / " + String::num_int64(gm->get_player_next_exp());
        exp_label->set_text(exp_text);
    }
}

// --- セッター・ゲッターの実装 ---

void HUD::set_hp_bar_path(const NodePath& path)
{
    hp_bar_path = path;
}

NodePath HUD::get_hp_bar_path() const
{
    return hp_bar_path;
}

void HUD::set_level_label_path(const NodePath& path)
{
    level_label_path = path;
}

NodePath HUD::get_level_label_path() const
{
    return level_label_path;
}

void HUD::set_exp_label_path(const NodePath& path)
{
    exp_label_path = path;
}

NodePath HUD::get_exp_label_path() const
{
    return exp_label_path;
}