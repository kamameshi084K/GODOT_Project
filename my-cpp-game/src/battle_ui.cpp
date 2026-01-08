#include "battle_ui.hpp"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/button.hpp>

using namespace godot;

void BattleUI::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_attack_button_pressed"), &BattleUI::_on_attack_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_run_button_pressed"), &BattleUI::_on_run_button_pressed);
    
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

    // シーケンサー用のメソッドを登録
    ClassDB::bind_method(D_METHOD("seq_player_attack_start"), &BattleUI::seq_player_attack_start);
    ClassDB::bind_method(D_METHOD("seq_deal_damage"), &BattleUI::seq_deal_damage);
    ClassDB::bind_method(D_METHOD("seq_end_enemy_turn"), &BattleUI::seq_end_enemy_turn);
    ClassDB::bind_method(D_METHOD("show_message", "text"), &BattleUI::show_message);

    ClassDB::bind_method(D_METHOD("seq_enemy_attack_start"), &BattleUI::seq_enemy_attack_start);
    ClassDB::bind_method(D_METHOD("seq_enemy_deal_damage"), &BattleUI::seq_enemy_deal_damage);
    ClassDB::bind_method(D_METHOD("seq_end_player_turn"), &BattleUI::seq_end_player_turn);
}

BattleUI::BattleUI()
{
    enemy_hp = 3;
    max_player_hp = 10; // 最大HP
    player_hp = max_player_hp;
    enemy_anim = nullptr;
    player_anim = nullptr; // 初期化
    player_hp_bar = nullptr; // 初期化
}

BattleUI::~BattleUI() {}

void BattleUI::_ready()
{
    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);

    // 敵のアニメ取得
    if (!enemy_anim_path.is_empty())
    {
        enemy_anim = get_node<AnimationPlayer>(enemy_anim_path);
    }
    if (enemy_anim)
    {
        // 名前を変えたので注意！
        enemy_anim->connect("animation_finished", Callable(this, "_on_enemy_animation_finished"));
    }

    // ▼ 追加: プレイヤーのアニメ取得
    if (!player_anim_path.is_empty())
    {
        player_anim = get_node<AnimationPlayer>(player_anim_path);
    }
    if (player_anim)
    {
        // プレイヤーが終わった時のシグナルを接続
        player_anim->connect("animation_finished", Callable(this, "_on_player_animation_finished"));
    }

    // HPバー取得
    player_hp_bar = get_node<ProgressBar>("PlayerHPBar");
    if (player_hp_bar)
    {
        player_hp_bar->set_max(max_player_hp);
        player_hp_bar->set_value(player_hp);
    }
    // 「たたかう」ボタン取得＆フォーカス設定
    attack_button = get_node<Button>("CommandWindow/VBoxContainer/AttackButton");
    run_button = get_node<Button>("CommandWindow/VBoxContainer/RunButton");

    command_window = get_node<PanelContainer>("CommandWindow"); // コマンドウィンドウ取得
    message_window = get_node<PanelContainer>("MessageWindow"); // メッセージウィンドウ取得
    message_label = get_node<Label>("MessageWindow/MessageLabel"); // メッセージラベル取得

    sequencer = get_node<AnimationPlayer>("Sequencer");

    if (command_window)
    {
        // 最初はコマンド表示
        command_window->show();
    }

    if (message_window)
    {
        // 最初はメッセージ非表示
        message_window->hide();
    }
    
    // 最初はコマンド表示
    if(attack_button) 
    {
        attack_button->grab_focus();

    }
}

// ボタンを押した時の処理
void BattleUI::_on_attack_button_pressed()
{
    // ガード処理: 演出中ならボタンを押せない
    if (sequencer && sequencer->is_playing()) return;

    // C++側でタイミング制御せず、監督（シーケンサー）に任せる！
    if (sequencer)
    {
        // エディタで作ったアニメーション名を指定
        sequencer->play("PlayerAttackAction"); 
    }
}


void BattleUI::_on_run_button_pressed()
{
    if (sequencer && sequencer->is_playing()) return;

    show_message(String::utf8("プレイヤーは にげだした！"));
    
    // 即座に逃げる（本来はここもシーケンサーでやるとカッコいいですが今回は省略）
    get_tree()->change_scene_to_file("res://world.tscn");
}

// プレイヤーのアニメが終わった時（＝剣を振り終わった時）
void BattleUI::_on_player_animation_finished(const StringName &anim_name)
{
    // 攻撃や被弾モーションが終わったら、待機モーションに戻すだけにする
    if (anim_name == String("Sword_Attack") || anim_name == String("RecieveHit")) 
    {
        // 名前はエディタに合わせてください (例: "Idle_Attacking" or "Idle")
        if (player_anim) player_anim->play("Idle_Attacking"); 
    }
}

void BattleUI::_on_enemy_animation_finished(const StringName &anim_name)
{
    if (anim_name == String("Death"))
    {
        UtilityFunctions::print("You Win!");
        // 本来はここもシーケンサーで「勝利ファンファーレ→画面遷移」とやると良い
        get_tree()->change_scene_to_file("res://world.tscn");
    }
    else if (anim_name == String("HitReact"))
    {
        // 敵が痛がり終わったら反撃
        // (本来はここから「敵ターン演出」のシーケンサーを再生すべきですが、簡易的に直接再生します)
        UtilityFunctions::print("Enemy counter attacks!");
        sequencer->play("EnemyAttackAction");
    }
    else if (anim_name == String("Punch"))
    {
        // 単に待機に戻すだけでOK
        enemy_anim->play("Idle");
    }
}

void BattleUI::seq_player_attack_start()
{
    show_message(String::utf8("プレイヤーの こうげき！"));
    if (player_anim) player_anim->play("Sword_Attack"); 
}

void BattleUI::seq_deal_damage()
{
    enemy_hp--;
    show_message(String::utf8("モンスターに 1 のダメージ！"));

    if (enemy_hp > 0) {
        if (enemy_anim) enemy_anim->play("HitReact");
    } else {
        if (enemy_anim) enemy_anim->play("Death");
    }
}

void BattleUI::seq_end_player_turn()
{
    // プレイヤーのターン終了時に何かしたい場合はここに書く
    // 今回は「HitReact」の終了シグナルで敵ターンに移るので、ここは空でもOK
}

// --- 敵のターン ---

void BattleUI::seq_enemy_attack_start()
{
    show_message(String::utf8("モンスターの こうげき！"));
    if (enemy_anim) enemy_anim->play("Punch"); 
}

void BattleUI::seq_enemy_deal_damage()
{
    // ★実装: ここでHPを減らし、プレイヤーのリアクションを取る
    player_hp -= 2;
    if (player_hp_bar) player_hp_bar->set_value(player_hp);
    
    show_message(String::utf8("プレイヤーに 2 のダメージ！"));

    if (player_hp <= 0)
    {
        // 敗北処理
        get_tree()->change_scene_to_file("res://world.tscn");
    }
    else
    {
        // プレイヤーが痛がる
        if (player_anim) player_anim->play("RecieveHit");
    }
}

void BattleUI::seq_end_enemy_turn()
{
    // コマンド入力に戻す
    if (message_window) message_window->hide();
    if (command_window) command_window->show();
    
    if (attack_button) attack_button->grab_focus();
}

// 汎用メッセージ表示
void BattleUI::show_message(const String &text)
{
    if (command_window) command_window->hide();
    if (message_window) message_window->show();
    if (message_label) message_label->set_text(text);
}

void BattleUI::set_enemy_anim_path(const NodePath &path) { enemy_anim_path = path; }
NodePath BattleUI::get_enemy_anim_path() const { return enemy_anim_path; }
void BattleUI::set_player_anim_path(const NodePath &path) { player_anim_path = path; }
NodePath BattleUI::get_player_anim_path() const { return player_anim_path; }