#include "battle_ui.hpp"
#include "game_manager.hpp" 

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/node3d.hpp>

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
    
    // ---------------------------------------------------------
    // ▼▼▼ 敵の動的スポーン処理 ▼▼▼
    // ---------------------------------------------------------
    GameManager *gm = GameManager::get_singleton();
    AnimationPlayer *spawned_anim = nullptr; // 新しく生成した敵のアニメ

    if (gm)
    {
        enemy_hp = gm->get_next_enemy_max_hp();
        enemy_attack_power = gm->get_next_enemy_attack();
        enemy_name = gm->get_next_enemy_name();
        enemy_defense_power = gm->get_next_enemy_defense();

        player_attack_power = gm->get_player_attack();
        player_defense_power = gm->get_player_defense();

        max_player_hp = gm->get_player_max_hp(); 
        player_hp = gm->get_player_current_hp();

        String next_enemy = gm->get_next_enemy_scene_path();
        
        // もしGameManagerに「次の敵」がセットされていたら
        if (!next_enemy.is_empty())
        {
            // 1. 今バトルシーンに置いてある「仮の敵」を探す
            // ※ エディタ上で、敵を置く場所（Node3D）の名前を "EnemyPos" とかにしておくと便利ですが
            //    今回は enemy_anim_path の親ノードを「仮の敵」とみなして差し替えます。
            
            Node *placeholder_enemy = nullptr;
            if (!enemy_anim_path.is_empty())
            {
                AnimationPlayer *temp = get_node<AnimationPlayer>(enemy_anim_path);
                if (temp) placeholder_enemy = temp->get_parent(); // アニメプレイヤーの親＝敵モデル
            }

            // 2. 新しい敵を読み込む
            Ref<PackedScene> enemy_scene = ResourceLoader::get_singleton()->load(next_enemy);
            if (enemy_scene.is_valid())
            {
                Node *new_enemy_node = enemy_scene->instantiate();
                Node3D *new_enemy_3d = Object::cast_to<Node3D>(new_enemy_node);

                if (placeholder_enemy)
                {
                    // 仮の敵と同じ場所に置く
                    Node3D *placeholder_3d = Object::cast_to<Node3D>(placeholder_enemy);
                    if (placeholder_3d && new_enemy_3d)
                    {
                        new_enemy_3d->set_transform(placeholder_3d->get_transform());
                    }
                    
                    // 親ノード（Stageなど）に追加
                    placeholder_enemy->get_parent()->add_child(new_enemy_node);
                    
                    // 仮の敵を削除（さようなら）
                    placeholder_enemy->queue_free();
                }
                else
                {
                    // 仮の敵が見つからない場合はとりあえず自身の子として追加
                    add_child(new_enemy_node);
                }

                // 3. 新しい敵の中から AnimationPlayer を探す
                // ※生成した敵の中に "AnimationPlayer" という名前のノードがあると想定
                spawned_anim = nullptr;
                TypedArray<Node> children = new_enemy_node->find_children("*", "AnimationPlayer", true, false); 
                // ↑ "*"は名前不問、"AnimationPlayer"は型指定、trueは再帰検索

                if (children.size() > 0)
                {
                    spawned_anim = Object::cast_to<AnimationPlayer>(children[0]);
                }
            }
        }
    }
    else
    {
        // デフォルト値
        enemy_hp = 3;
        enemy_attack_power = 1;
        enemy_name = "Enemy";
    }
    

    // ---------------------------------------------------------
    // ▼▼▼ アニメーションプレイヤーの接続 ▼▼▼
    // ---------------------------------------------------------

    // 新しく生成されたアニメがあればそれを使い、なければエディタ設定のものを使う
    if (spawned_anim)
    {
        enemy_anim = spawned_anim;
    }
    else if (!enemy_anim_path.is_empty())
    {
        enemy_anim = get_node<AnimationPlayer>(enemy_anim_path);
    }

    if (enemy_anim)
    {
        enemy_anim->connect("animation_finished", Callable(this, "_on_enemy_animation_finished"));
        // 登場時にIdleを再生しておく
        enemy_anim->play("Idle");
    }

    // HPバー取得
    player_hp_bar = get_node<ProgressBar>("PlayerHPBar");
    if (player_hp_bar)
    {
        player_hp_bar->set_max(max_player_hp);
        player_hp_bar->set_value(player_hp);
    }

    if (!player_anim_path.is_empty())
    {
        player_anim = get_node<AnimationPlayer>(player_anim_path);
        if (player_anim)
        {
            player_anim->connect("animation_finished", Callable(this, "_on_player_animation_finished"));
        }
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
    
    GameManager *gm = GameManager::get_singleton();
    if (gm)
    {
        String return_path = gm->get_last_scene_path();
        if (!return_path.is_empty())
        {
            get_tree()->change_scene_to_file(return_path);
        }
        else
        {
            // 念のため、パスが取れなかったら world.tscn へ
            get_tree()->change_scene_to_file("res://world.tscn");
        }
    }
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
        GameManager *gm = GameManager::get_singleton();
        String return_path = "res://world.tscn";
        if (gm)
        {
            String current_id = gm->get_current_enemy_id();
            if (!current_id.is_empty())
            {
                gm->add_defeated_enemy(current_id);
                // 次の戦いのためにリセットしておく（任意）
                gm->set_current_enemy_id(""); 
            }
            int reward = gm->get_next_enemy_exp_reward();
            gm->gain_experience(reward);

            Ref<MonsterData> new_monster;
            new_monster.instantiate();
            
            // 名前とステータスを保存
            new_monster->set_monster_name(gm->get_next_enemy_name());
            // ※ここでは敵のステータスをそのまま使っていますが、乱数を入れたりしても面白いです
            new_monster->set_stats(
                gm->get_next_enemy_max_hp(), 
                gm->get_next_enemy_attack(), 
                gm->get_next_enemy_defense(), 
                5 // 素早さ（とりあえず固定値）
            );
            
            // マネージャーに追加（パーティor倉庫へ）
            gm->add_monster(new_monster);

            // 戻り先を取得
            String saved_path = gm->get_last_scene_path();
            if (!saved_path.is_empty())
            {
                return_path = saved_path;
            }
        }
        // 本来はここもシーケンサーで「勝利ファンファーレ→画面遷移」とやると良い
        get_tree()->change_scene_to_file(return_path);
    }
    else if (anim_name == String("HitReact"))
    {
        // 敵が痛がり終わったら反撃
        // (本来はここから「敵ターン演出」のシーケンサーを再生すべきですが、簡易的に直接再生します)
        UtilityFunctions::print("Enemy counter attacks!");
        sequencer->play("EnemyAttackAction");
    }
    else if (anim_name == String("Attack"))
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

void BattleUI::seq_deal_damage() // プレイヤーの攻撃
{
    // ダメージ計算: (プレイヤー攻撃 - 敵防御) ※最低1
    int damage = player_attack_power - enemy_defense_power;
    if (damage < 1)
    {
        damage = 1;
    }

    enemy_hp -= damage;
    
    // メッセージ更新
    show_message(enemy_name + String::utf8(" に ") + String::num_int64(damage) + String::utf8(" のダメージ！"));

    if (enemy_hp > 0)
    {
        if (enemy_anim)
        {
            enemy_anim->play("HitReact");
        }
    }
    else
    {
        if (enemy_anim)
        {
            enemy_anim->play("Death");
        }
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
    show_message(enemy_name + String::utf8(" の こうげき！"));
    if (enemy_anim) 
    {
        enemy_anim->play("Attack");
    }
}

void BattleUI::seq_enemy_deal_damage()
{
    int damage = enemy_attack_power - player_defense_power;
    if (damage < 1)
    {
        damage = 1;
    }
    player_hp -= damage;

    if (player_hp_bar) 
    {
        player_hp_bar->set_value(player_hp);
    }
    show_message(String::utf8("プレイヤーに ") + String::num_int64(damage) + String::utf8(" のダメージ！"));

    if (player_hp <= 0)
    {
        get_tree()->change_scene_to_file("res://world.tscn");
    }
    else
    {
        if (player_anim)
        {
            player_anim->play("RecieveHit");
        }
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