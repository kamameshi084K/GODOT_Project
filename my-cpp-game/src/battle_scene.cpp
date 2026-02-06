#include "battle_scene.hpp"
#include "game_manager.hpp"
#include "monster_data.hpp"
#include "skill_data.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/animation_player.hpp>

using namespace godot;

void BattleScene::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_rpc_notify_loaded"), &BattleScene::_rpc_notify_loaded);
    ClassDB::bind_method(D_METHOD("_rpc_start_spawning"), &BattleScene::_rpc_start_spawning);
    ClassDB::bind_method(D_METHOD("_rpc_spawn_enemy", "monster_name", "is_player"), &BattleScene::_rpc_spawn_enemy);
    
    ClassDB::bind_method(D_METHOD("_on_skill_1_pressed"), &BattleScene::_on_skill_1_pressed);
    ClassDB::bind_method(D_METHOD("_on_skill_2_pressed"), &BattleScene::_on_skill_2_pressed);
    ClassDB::bind_method(D_METHOD("_on_skill_3_pressed"), &BattleScene::_on_skill_3_pressed);
    
    ClassDB::bind_method(D_METHOD("_rpc_submit_hand", "hand"), &BattleScene::_rpc_submit_hand);
    ClassDB::bind_method(D_METHOD("_rpc_resolve_janken", "h_hand", "c_hand", "winner_side", "first_attacker"), &BattleScene::_rpc_resolve_janken);
    ClassDB::bind_method(D_METHOD("_rpc_notify_defeat"), &BattleScene::_rpc_notify_defeat);
}

BattleScene::BattleScene()
{
    player_spawn_pos = nullptr;
    enemy_spawn_pos = nullptr;
    skill_button_1 = nullptr;
    skill_button_2 = nullptr;
    skill_button_3 = nullptr;
    
    loaded_player_count = 0;
    player_hp = 10;
    enemy_hp = 10;
    
    has_selected = false;
    command_received_count = 0;

    server_host_hand = "";
    server_client_hand = "";

    Dictionary rpc_config_any;
    rpc_config_any["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    rpc_config_any["call_local"] = false;
    rpc_config_any["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;

    Dictionary rpc_config_auth;
    rpc_config_auth["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    rpc_config_auth["call_local"] = true; // 自分自身でも実行されるようにする
    rpc_config_auth["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    
    Dictionary rpc_config_loaded = rpc_config_any;
    rpc_config_loaded["call_local"] = true;
    rpc_config("_rpc_notify_loaded", rpc_config_loaded);

    rpc_config("_rpc_start_spawning", rpc_config_auth);
    rpc_config("_rpc_spawn_enemy", rpc_config_any);
    
    rpc_config("_rpc_submit_hand", rpc_config_any);
    rpc_config("_rpc_resolve_janken", rpc_config_auth);
    rpc_config("_rpc_notify_defeat", rpc_config_any);
}

BattleScene::~BattleScene()
{
}

void BattleScene::_ready()
{
    Node* p_node = find_child("PlayerSpawnPos");
    if (p_node)
    {
        player_spawn_pos = Object::cast_to<Marker3D>(p_node);
    }
    Node* e_node = find_child("EnemySpawnPos");
    if (e_node)
    {
        enemy_spawn_pos = Object::cast_to<Marker3D>(e_node);
    }
    Node* b1 = find_child("SkillButton1");
    if (b1)
    {
        skill_button_1 = Object::cast_to<Button>(b1);
    }
    
    Node* b2 = find_child("SkillButton2");
    if (b2)
    {
        skill_button_2 = Object::cast_to<Button>(b2);
    }
    
    Node* b3 = find_child("SkillButton3");
    if (b3)
    {
        skill_button_3 = Object::cast_to<Button>(b3);
    }

    // ボタンにシグナルを接続 (エディタで接続していない場合、ここですると確実です)
    if (skill_button_1 && !skill_button_1->is_connected("pressed", Callable(this, "_on_skill_1_pressed")))
    {
        skill_button_1->connect("pressed", Callable(this, "_on_skill_1_pressed"));
    }
    if (skill_button_2 && !skill_button_2->is_connected("pressed", Callable(this, "_on_skill_2_pressed")))
    {
        skill_button_2->connect("pressed", Callable(this, "_on_skill_2_pressed"));
    }
    if (skill_button_3 && !skill_button_3->is_connected("pressed", Callable(this, "_on_skill_3_pressed")))
    {
        skill_button_3->connect("pressed", Callable(this, "_on_skill_3_pressed"));
    }

    // 初期状態は無効化
    if(skill_button_1)
    {
        skill_button_1->set_disabled(true);
    }
    if(skill_button_2)
    {
        skill_button_2->set_disabled(true);
    }
    if(skill_button_3)
    {
        skill_button_3->set_disabled(true);
    }
    rpc("_rpc_notify_loaded");
}

void BattleScene::_rpc_notify_loaded()
{
    if (!get_tree()->get_multiplayer()->is_server()) return;
    
    loaded_player_count++;
    int total = get_tree()->get_multiplayer()->get_peers().size() + 1;
    
    if (loaded_player_count >= total)
    {
        rpc("_rpc_start_spawning");
    }
}

void BattleScene::_rpc_start_spawning()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        player_hp = gm->get_player_max_hp();
        enemy_hp = gm->get_next_enemy_max_hp(); 
        
        // --- 自分のモンスターの表示 ---
        TypedArray<MonsterData> party = gm->get_party();
        if (party.size() > 0)
        {
            Ref<MonsterData> leader = party[0];
            
            if (leader.is_valid())
            {
                // ★修正: 名前ではなく、データ内のパスを取得する
                String model_path = leader->get_model_path();
                
                current_skills = leader->get_skills();
                _update_ui_buttons();

                // パスを渡してスポーンさせる
                _rpc_spawn_enemy(model_path, true); 
                rpc("_rpc_spawn_enemy", model_path, true);
            }
        }

        // --- ★追加: 敵モンスターの表示 ---
        // サーバー側（ホスト）が責任を持って敵情報を取得し、全員に通知する
        if (get_tree()->get_multiplayer()->is_server())
        {
            Ref<MonsterData> enemy_data = gm->get_next_enemy_data();
            if (enemy_data.is_valid())
            {
                String enemy_path = enemy_data->get_model_path();
                
                // 自分（ホスト）の画面に出す
                _rpc_spawn_enemy(enemy_path, false);
                // 相手（クライアント）の画面にも出す
                rpc("_rpc_spawn_enemy", enemy_path, false);
            }
        }
    }

    has_selected = false;
    if (skill_button_1) skill_button_1->set_disabled(false);
    if (skill_button_2) skill_button_2->set_disabled(false);
    if (skill_button_3) skill_button_3->set_disabled(false);
    
    UtilityFunctions::print("Janken Start with Skills and Models!");
}

void BattleScene::_rpc_spawn_enemy(const String& model_path, bool is_player)
{
    // ★修正: _get_model_path_by_name を通さず、直接ロードする
    // String path = _get_model_path_by_name(monster_name); 
    
    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(model_path);
    
    if (scene.is_valid())
    {
        Node* instance = scene->instantiate();
        Node3D* model_3d = Object::cast_to<Node3D>(instance);
        
        Marker3D* target_pos = is_player ? player_spawn_pos : enemy_spawn_pos;
        
        if (target_pos && model_3d)
        {
            for (int i = 0; i < target_pos->get_child_count(); ++i)
            {
                target_pos->get_child(i)->queue_free();
            }
            
            if (is_player)
            {
                model_3d->set_rotation_degrees(Vector3(0, 180, 0));
            }
            else
            {
                model_3d->set_rotation_degrees(Vector3(0, 0, 0));
            }

            AnimationPlayer* anim = Object::cast_to<AnimationPlayer>(model_3d->find_child("AnimationPlayer", true, false));
            if (anim)
            {
                if (anim->has_animation("Idle")) anim->play("Idle");
                else if (anim->has_animation("idle")) anim->play("idle");
            }
            
            target_pos->call_deferred("add_child", model_3d);
            UtilityFunctions::print("Spawned Model: ", model_path);
        }
    }
    else
    {
        UtilityFunctions::print("Error: Failed to load model at ", model_path);
    }
}

String BattleScene::_get_model_path_by_name(const String& name)
{
    // 将来的には name に応じてパスを変える
    return "res://scenes/battler_bird.tscn"; 
}

void BattleScene::_on_skill_1_pressed()
{
    if (current_skills.size() > 0)
    {
        Ref<SkillData> skill = current_skills[0];
        if (skill.is_valid())
        {
            String hand = _hand_type_to_string(skill->get_hand_type());
            UtilityFunctions::print("Used Skill 1: ", skill->get_skill_name());
            _submit_hand(hand);
        }
    }
}

void BattleScene::_on_skill_2_pressed()
{
    if (current_skills.size() > 1)
    {
        Ref<SkillData> skill = current_skills[1];
        if (skill.is_valid())
        {
            String hand = _hand_type_to_string(skill->get_hand_type());
            UtilityFunctions::print("Used Skill 2: ", skill->get_skill_name());
            _submit_hand(hand);
        }
    }
}

void BattleScene::_on_skill_3_pressed()
{
    if (current_skills.size() > 2)
    {
        Ref<SkillData> skill = current_skills[2];
        if (skill.is_valid())
        {
            String hand = _hand_type_to_string(skill->get_hand_type());
            UtilityFunctions::print("Used Skill 3: ", skill->get_skill_name());
            _submit_hand(hand);
        }
    }
}

void BattleScene::_submit_hand(const String& hand)
{
    if(has_selected) return;
    has_selected = true;
    
    // ★修正: 新しいボタンを無効化
    if(skill_button_1) skill_button_1->set_disabled(true);
    if(skill_button_2) skill_button_2->set_disabled(true);
    if(skill_button_3) skill_button_3->set_disabled(true);
    
    if (get_tree()->get_multiplayer()->is_server())
    {
        _rpc_submit_hand(hand);
    }
    else
    {
        rpc_id(1, "_rpc_submit_hand", hand);
    }
}

void BattleScene::_rpc_submit_hand(const String& hand)
{
    if (!get_tree()->get_multiplayer()->is_server()) return;

    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    
    if (sender_id == 1 || sender_id == 0) server_host_hand = hand;
    else server_client_hand = hand;
    
    command_received_count++;
    int total = get_tree()->get_multiplayer()->get_peers().size() + 1;
    
    if (command_received_count >= total)
    {
        command_received_count = 0; 
        
        int winner_side = 0;
        if (server_host_hand == server_client_hand) winner_side = 0;
        else if ((server_host_hand == "rock" && server_client_hand == "scissors") ||
                 (server_host_hand == "scissors" && server_client_hand == "paper") ||
                 (server_host_hand == "paper" && server_client_hand == "rock"))
        {
            winner_side = 1;
        }
        else 
        {
            winner_side = 2;
        }

        int first_attacker = 1; 
        GameManager* gm = GameManager::get_singleton();
        if (gm && winner_side == 0)
        {
            if (gm->get_next_enemy_speed() > gm->get_player_speed()) first_attacker = 2;
        }

        rpc("_rpc_resolve_janken", server_host_hand, server_client_hand, winner_side, first_attacker);
        server_host_hand = "";
        server_client_hand = "";
    }
}

void BattleScene::_rpc_resolve_janken(const String& h_hand, const String& c_hand, int winner_side, int first_attacker)
{
    bool is_server = get_tree()->get_multiplayer()->is_server();
    
    // GameMangerからステータスを取得
    GameManager* gm = GameManager::get_singleton();
    int my_atk = gm->get_player_attack();
    int my_def = gm->get_player_defense();
    int enemy_atk = gm->get_next_enemy_attack();
    int enemy_def = gm->get_next_enemy_defense();

    // 最低保証ダメージ（防御が高すぎても1は通るようにする）
    // 計算式: 攻撃力 - 防御力
    int damage_to_enemy = MAX(1, my_atk - enemy_def);
    int damage_to_player = MAX(1, enemy_atk - my_def);

    if (winner_side != 0)
    {
        // どちらかが勝った場合
        bool i_won = (is_server && winner_side == 1) || (!is_server && winner_side == 2);
        
        if (i_won)
        {
            // 自分の勝ち -> 敵にダメージ
            enemy_hp -= damage_to_enemy;
            UtilityFunctions::print("Hit! Damage: ", damage_to_enemy, " (Enemy HP: ", enemy_hp, ")");
        }
        else
        {
            // 自分の負け -> 自分にダメージ
            player_hp -= damage_to_player;
            UtilityFunctions::print("Ouch! Took: ", damage_to_player, " (Player HP: ", player_hp, ")");
        }
    }
    else
    {
        // あいこの場合（素早さ判定）
        bool am_i_first = (is_server && first_attacker == 1) || (!is_server && first_attacker == 2);
        
        if (am_i_first)
        {
             // 自分が先制攻撃
             enemy_hp -= damage_to_enemy;
             UtilityFunctions::print("Draw but Fast! Hit: ", damage_to_enemy);
             
             // 敵が生きていれば反撃（ターン制っぽくするなら）
             // 今回は「あいこは速い方が一方的に殴れる」または「両方殴る」など仕様次第ですが、
             // 一旦「速い方だけ攻撃」のままにしておきます
        }
        else 
        {
             // 敵が先制攻撃
             player_hp -= damage_to_player;
             UtilityFunctions::print("Draw and Slow... Took: ", damage_to_player);
        }
    }

    // --- 以下、勝敗判定とシーン遷移（変更なし） ---

    if (player_hp <= 0)
    {
        player_hp = 0;
        if (is_server) rpc("_rpc_notify_defeat");
        
        // 負けたので町に戻る
        UtilityFunctions::print("DEFEATED...");
        get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
    }
    else if (enemy_hp <= 0)
    {
        enemy_hp = 0;
        UtilityFunctions::print("VICTORY! Returning to town...");
        
        // 経験値獲得などの処理をここに挟むことも可能です
        if(gm) gm->gain_experience(gm->get_next_enemy_exp_reward());

        get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
    }
    else
    {
        // 次のターンへ
        has_selected = false;
        _update_ui_buttons(); // ボタンを再有効化
    }
}

void BattleScene::_rpc_notify_defeat()
{
    UtilityFunctions::print("VICTORY! Returning to town...");
    get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
}

void BattleScene::_update_ui_buttons()
{
    // スキル1
    if (skill_button_1)
    {
        if (current_skills.size() > 0)
        {
            Ref<SkillData> skill = current_skills[0];
            if (skill.is_valid())
            {
                String type_str = _hand_type_to_string(skill->get_hand_type());
                skill_button_1->set_text(skill->get_skill_name() + "\n(" + type_str + ")");
                skill_button_1->set_disabled(false);
            }
        }
        else
        {
            skill_button_1->set_text("- No Skill -");
            skill_button_1->set_disabled(true);
        }
    }

    // スキル2
    if (skill_button_2)
    {
        if (current_skills.size() > 1)
        {
            Ref<SkillData> skill = current_skills[1];
            if (skill.is_valid())
            {
                String type_str = _hand_type_to_string(skill->get_hand_type());
                skill_button_2->set_text(skill->get_skill_name() + "\n(" + type_str + ")");
                skill_button_2->set_disabled(false);
            }
        }
        else
        {
            skill_button_2->set_text("- Empty -");
            skill_button_2->set_disabled(true);
        }
    }

    // スキル3
    if (skill_button_3)
    {
        if (current_skills.size() > 2)
        {
            Ref<SkillData> skill = current_skills[2];
            if (skill.is_valid())
            {
                String type_str = _hand_type_to_string(skill->get_hand_type());
                skill_button_3->set_text(skill->get_skill_name() + "\n(" + type_str + ")");
                skill_button_3->set_disabled(false);
            }
        }
        else
        {
            skill_button_3->set_text("- Empty -");
            skill_button_3->set_disabled(true);
        }
    }
}

// UI更新などで使うヘルパー関数
String BattleScene::_hand_type_to_string(int type)
{
    switch(type)
    {
        case 0: return "rock";     // SkillData::HAND_ROCK
        case 1: return "scissors"; // SkillData::HAND_SCISSORS
        case 2: return "paper";    // SkillData::HAND_PAPER
        default: return "rock";
    }
}