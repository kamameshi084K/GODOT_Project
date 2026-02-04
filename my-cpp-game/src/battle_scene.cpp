#include "battle_scene.hpp"
#include "game_manager.hpp"
#include "monster_data.hpp"

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
    
    ClassDB::bind_method(D_METHOD("_on_rock_pressed"), &BattleScene::_on_rock_pressed);
    ClassDB::bind_method(D_METHOD("_on_scissors_pressed"), &BattleScene::_on_scissors_pressed);
    ClassDB::bind_method(D_METHOD("_on_paper_pressed"), &BattleScene::_on_paper_pressed);
    
    ClassDB::bind_method(D_METHOD("_rpc_submit_hand", "hand"), &BattleScene::_rpc_submit_hand);
    ClassDB::bind_method(D_METHOD("_rpc_resolve_janken", "h_hand", "c_hand", "winner_side", "first_attacker"), &BattleScene::_rpc_resolve_janken);
    ClassDB::bind_method(D_METHOD("_rpc_notify_defeat"), &BattleScene::_rpc_notify_defeat);
}

BattleScene::BattleScene()
{
    player_spawn_pos = nullptr;
    enemy_spawn_pos = nullptr;
    rock_button = nullptr;
    scissors_button = nullptr;
    paper_button = nullptr;
    
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
    if (p_node) player_spawn_pos = Object::cast_to<Marker3D>(p_node);
    Node* e_node = find_child("EnemySpawnPos");
    if (e_node) enemy_spawn_pos = Object::cast_to<Marker3D>(e_node);

    Node* b1 = find_child("RockButton");
    if (b1) rock_button = Object::cast_to<Button>(b1);
    Node* b2 = find_child("ScissorsButton");
    if (b2) scissors_button = Object::cast_to<Button>(b2);
    Node* b3 = find_child("PaperButton");
    if (b3) paper_button = Object::cast_to<Button>(b3);

    if(rock_button) rock_button->set_disabled(true);
    if(scissors_button) scissors_button->set_disabled(true);
    if(paper_button) paper_button->set_disabled(true);

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
    // この関数は AUTHORITY (サーバー) から全員に RPC で呼ばれます
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        player_hp = gm->get_player_max_hp();
        enemy_hp = gm->get_next_enemy_max_hp(); 
        
        TypedArray<MonsterData> party = gm->get_party();
        if (party.size() > 0)
        {
            Ref<MonsterData> leader = party[0];
            String my_monster_name = leader->get_monster_name();
            
            // 自分のモンスターを自分の「PlayerPos」に配置
            _rpc_spawn_enemy(my_monster_name, true); 

            // 相手の画面では、自分のモンスターを「EnemyPos」に配置させる
            rpc("_rpc_spawn_enemy", my_monster_name, false);
        }
    }

    has_selected = false;
    if (rock_button) rock_button->set_disabled(false);
    if (scissors_button) scissors_button->set_disabled(false);
    if (paper_button) paper_button->set_disabled(false);
    
    UtilityFunctions::print("Janken Start!");
}

void BattleScene::_rpc_spawn_enemy(const String& monster_name, bool is_player)
{
    String path = _get_model_path_by_name(monster_name);
    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
    
    if (scene.is_valid())
    {
        Node* instance = scene->instantiate();
        Node3D* model_3d = Object::cast_to<Node3D>(instance);
        
        // 配置場所の決定
        Marker3D* target_pos = is_player ? player_spawn_pos : enemy_spawn_pos;
        
        if (target_pos && model_3d)
        {
            // 1. 古いモデルの削除
            for (int i = 0; i < target_pos->get_child_count(); ++i)
            {
                target_pos->get_child(i)->queue_free();
            }
            
            // 2. 向きの調整
            // プレイヤーは敵（奥）を向き、敵はプレイヤー（手前）を向くように設定
            // モデルの初期方向が「正面」を向いていると仮定して調整します
            if (is_player)
            {
                // プレイヤー側：そのまま、あるいは必要に応じて180度回転
                model_3d->set_rotation_degrees(Vector3(0, 180, 0));
            }
            else
            {
                // 敵側：プレイヤーの方を向くように180度回転
                model_3d->set_rotation_degrees(Vector3(0, 0, 0));
            }

            // 3. アニメーションの再生
            // モデル内のどこかにある AnimationPlayer を探す
            // "AnimationPlayer" という名前で配置されていることを想定
            AnimationPlayer* anim = Object::cast_to<AnimationPlayer>(model_3d->find_child("AnimationPlayer", true, false));
            if (anim)
            {
                // 待機モーション（例: "Idle" や "idle"）をループ再生
                if (anim->has_animation("Idle"))
                {
                    anim->play("Idle");
                }
                else if (anim->has_animation("idle"))
                {
                    anim->play("idle");
                }
            }
            
            target_pos->call_deferred("add_child", model_3d);
            UtilityFunctions::print("Spawned and Animated: ", monster_name);
        }
    }
}

String BattleScene::_get_model_path_by_name(const String& name)
{
    // 将来的には name に応じてパスを変える
    return "res://battler_bird.tscn"; 
}

void BattleScene::_on_rock_pressed()     { _submit_hand("rock"); }
void BattleScene::_on_scissors_pressed() { _submit_hand("scissors"); }
void BattleScene::_on_paper_pressed()    { _submit_hand("paper"); }

void BattleScene::_submit_hand(const String& hand)
{
    if(has_selected) return;
    has_selected = true;
    
    if(rock_button) rock_button->set_disabled(true);
    if(scissors_button) scissors_button->set_disabled(true);
    if(paper_button) paper_button->set_disabled(true);
    
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
    int damage_val = 100; // テスト用

    if (winner_side != 0)
    {
        bool i_won = (is_server && winner_side == 1) || (!is_server && winner_side == 2);
        if (i_won) enemy_hp -= damage_val;
        else player_hp -= damage_val;
    }
    else
    {
        bool am_i_first = (is_server && first_attacker == 1) || (!is_server && first_attacker == 2);
        if (am_i_first) enemy_hp -= damage_val;
        else player_hp -= damage_val;

        if (player_hp > 0 && enemy_hp > 0)
        {
            if (am_i_first) player_hp -= damage_val;
            else enemy_hp -= damage_val;
        }
    }

    if (player_hp <= 0)
    {
        player_hp = 0;
        if (is_server) rpc("_rpc_notify_defeat");
        get_tree()->call_deferred("change_scene_to_file", "res://town.tscn");
    }
    else if (enemy_hp <= 0)
    {
        enemy_hp = 0;
        UtilityFunctions::print("I WON! Returning to town...");
        get_tree()->call_deferred("change_scene_to_file", "res://town.tscn");
    }
    else
    {
        has_selected = false;
        if (rock_button) rock_button->set_disabled(false);
        if (scissors_button) scissors_button->set_disabled(false);
        if (paper_button) paper_button->set_disabled(false);
    }
}

void BattleScene::_rpc_notify_defeat()
{
    UtilityFunctions::print("VICTORY! Returning to town...");
    get_tree()->call_deferred("change_scene_to_file", "res://town.tscn");
}