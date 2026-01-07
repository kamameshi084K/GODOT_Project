#include "battle_ui.hpp"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void BattleUI::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_attack_button_pressed"), &BattleUI::_on_attack_button_pressed);
    
    // コールバックを2つに分けました
    ClassDB::bind_method(D_METHOD("_on_enemy_animation_finished", "anim_name"), &BattleUI::_on_enemy_animation_finished);
    ClassDB::bind_method(D_METHOD("_on_player_animation_finished", "anim_name"), &BattleUI::_on_player_animation_finished);

    ClassDB::bind_method(D_METHOD("set_enemy_anim_path", "path"), &BattleUI::set_enemy_anim_path);
    ClassDB::bind_method(D_METHOD("get_enemy_anim_path"), &BattleUI::get_enemy_anim_path);

    ClassDB::bind_method(D_METHOD("set_player_anim_path", "path"), &BattleUI::set_player_anim_path);
    ClassDB::bind_method(D_METHOD("get_player_anim_path"), &BattleUI::get_player_anim_path);

    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "enemy_anim_path"),"set_enemy_anim_path", "get_enemy_anim_path");
    // ▼ 追加: インスペクタにプレイヤーのパス設定欄を作る
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "player_anim_path"),"set_player_anim_path", "get_player_anim_path");
}

BattleUI::BattleUI()
{
    enemy_hp = 3;
    player_hp = 10;
    enemy_anim = nullptr;
    player_anim = nullptr; // 初期化
    hp_label = nullptr;
}

BattleUI::~BattleUI() {}

void BattleUI::_ready()
{
    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);

    // 敵のアニメ取得
    if (!enemy_anim_path.is_empty()) {
        enemy_anim = get_node<AnimationPlayer>(enemy_anim_path);
    }
    if (enemy_anim) {
        // 名前を変えたので注意！
        enemy_anim->connect("animation_finished", Callable(this, "_on_enemy_animation_finished"));
    }

    // ▼ 追加: プレイヤーのアニメ取得
    if (!player_anim_path.is_empty()) {
        player_anim = get_node<AnimationPlayer>(player_anim_path);
    }
    if (player_anim) {
        // プレイヤーが終わった時のシグナルを接続
        player_anim->connect("animation_finished", Callable(this, "_on_player_animation_finished"));
    }

    hp_label = get_node<Label>("HPLabel");
    update_hp_label();
}

void BattleUI::update_hp_label() {
    if (hp_label) hp_label->set_text("HP: " + String::num_int64(player_hp));
}

// ボタンを押した時の処理
void BattleUI::_on_attack_button_pressed()
{
    // ガード処理: どっちかが動いてたら押せない
    if (enemy_anim && enemy_anim->is_playing() && enemy_anim->get_current_animation() != String("Idle")) return;
    if (player_anim && player_anim->is_playing() && player_anim->get_current_animation() != String("Idle_Attacking")) return;

    if (player_anim) {
        // ★まずプレイヤーが振る！
        // ※KayKitのアニメ名を確認して変えてください。 "Attack(1h)" とかかも？
        player_anim->play("Sword_Attack"); 
        UtilityFunctions::print("Player attacks...");
    }
}

// プレイヤーのアニメが終わった時（＝剣を振り終わった時）
void BattleUI::_on_player_animation_finished(const StringName &anim_name)
{
    // 攻撃モーションが終わったら、敵にダメージを与える
    // ※ここもアニメ名を確認して合わせる
    if (anim_name == String("Sword_Attack")) 
    {
        // プレイヤーは待機に戻る
        player_anim->play("Idle_Attacking");

        // ここから敵へのダメージ処理
        if (!enemy_anim) return;
        if (enemy_hp <= 0) return;

        enemy_hp--;
        UtilityFunctions::print("Hit! Enemy HP: ", enemy_hp);

        if (enemy_hp > 0) {
            enemy_anim->play("HitReact");
        } else {
            enemy_anim->play("Death");
        }
    }
}

// 敵のアニメが終わった時
void BattleUI::_on_enemy_animation_finished(const StringName &anim_name)
{
    // 敵が死んだら勝利
    if (anim_name == String("Death")) {
        UtilityFunctions::print("You Win!");
        get_tree()->change_scene_to_file("res://dungeon.tscn");
    }
    // 敵が痛がり終わったら → 反撃開始
    else if (anim_name == String("HitReact")) {
        UtilityFunctions::print("Enemy counter attacks!");
        enemy_anim->play("Attack");
    }
    // 敵の攻撃が終わったら → プレイヤーダメージ
    else if (anim_name == String("Attack")) {
        player_hp -= 2;
        update_hp_label();
        
        if (player_hp <= 0) {
            UtilityFunctions::print("Game Over...");
            get_tree()->change_scene_to_file("res://dungeon.tscn");
        } else {
            enemy_anim->play("Idle");
        }
    }
}

void BattleUI::set_enemy_anim_path(const NodePath &path) { enemy_anim_path = path; }
NodePath BattleUI::get_enemy_anim_path() const { return enemy_anim_path; }
void BattleUI::set_player_anim_path(const NodePath &path) { player_anim_path = path; }
NodePath BattleUI::get_player_anim_path() const { return player_anim_path; }