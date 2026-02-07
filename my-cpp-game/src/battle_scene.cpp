#include "battle_scene.hpp"
#include "game_manager.hpp"
#include "monster_data.hpp"
#include "skill_data.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/property_tweener.hpp>
#include <godot_cpp/classes/callback_tweener.hpp>
#include <godot_cpp/classes/interval_tweener.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void BattleScene::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_rpc_notify_loaded"), &BattleScene::_rpc_notify_loaded);
    ClassDB::bind_method(D_METHOD("_rpc_start_spawning"), &BattleScene::_rpc_start_spawning);
    ClassDB::bind_method(D_METHOD("_rpc_register_player_data", "peer_id", "model_path", "hp", "speed"), &BattleScene::_rpc_register_player_data);
    ClassDB::bind_method(D_METHOD("_rpc_setup_battle", "host_info", "client_info"), &BattleScene::_rpc_setup_battle);

    
    ClassDB::bind_method(D_METHOD("_on_skill_1_pressed"), &BattleScene::_on_skill_1_pressed);
    ClassDB::bind_method(D_METHOD("_on_skill_2_pressed"), &BattleScene::_on_skill_2_pressed);
    ClassDB::bind_method(D_METHOD("_on_skill_3_pressed"), &BattleScene::_on_skill_3_pressed);
    
    ClassDB::bind_method(D_METHOD("_rpc_submit_hand", "hand"), &BattleScene::_rpc_submit_hand);
    ClassDB::bind_method(D_METHOD("_rpc_resolve_janken", "h_hand", "c_hand", "winner_side", "first_attacker"), &BattleScene::_rpc_resolve_janken);
    ClassDB::bind_method(D_METHOD("_rpc_notify_defeat"), &BattleScene::_rpc_notify_defeat);

    ClassDB::bind_method(D_METHOD("request_battle_data"), &BattleScene::request_battle_data);
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
    rpc_config_auth["call_local"] = true; 
    rpc_config_auth["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    
    // RPC登録
    rpc_config("_rpc_notify_loaded", rpc_config_any);
    rpc_config("_rpc_register_player_data", rpc_config_any); // 誰でも送れる
    rpc_config("_rpc_setup_battle", rpc_config_auth);        // ホストだけが命令できる
    rpc_config("_rpc_submit_hand", rpc_config_any);
    rpc_config("_rpc_resolve_janken", rpc_config_auth);
    rpc_config("_rpc_notify_defeat", rpc_config_any);
}

BattleScene::~BattleScene()
{
}

void BattleScene::_ready()
{
    if (Engine::get_singleton()->is_editor_hint()) return;
    player_spawn_pos = get_node<Marker3D>("PlayerSpawnPos");
    enemy_spawn_pos = get_node<Marker3D>("EnemySpawnPos");

    // マーカーが取れているか確認用ログ（出力パネルで確認してください）
    if (player_spawn_pos) UtilityFunctions::print("Check: PlayerSpawnPos found.");
    else UtilityFunctions::print("Error: PlayerSpawnPos NOT found!");

    if (enemy_spawn_pos) UtilityFunctions::print("Check: EnemySpawnPos found.");
    else UtilityFunctions::print("Error: EnemySpawnPos NOT found!");
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
    // 1. まず自分の情報をサーバーに登録する
    _rpc_notify_loaded();

    // 2. サーバーに対して「準備ができたのでセットアップ情報をください」とリクエスト
    // call_deferred を使うことで、ノードが完全に登録された次のフレームで実行され、より確実になります
    call_deferred("request_battle_data");
}

void BattleScene::_rpc_notify_loaded()
{
    if (Engine::get_singleton()->is_editor_hint()) return;
    if (!is_inside_tree()) return; // シーンツリーにいないなら中断

    GameManager* gm = GameManager::get_singleton();
    if (!gm) return;

    // get_tree() が nullptr でないことを確認してからアクセス
    SceneTree* tree = get_tree();
    if (!tree || !tree->get_multiplayer().is_valid()) return;

    int my_id = tree->get_multiplayer()->get_unique_id();
    TypedArray<MonsterData> party = gm->get_party();
    
    // ★修正: 初期値を空ではなく、確実に存在するパスにしておく（テスト用）
    // あなたのプロジェクトにある実在するパス（例: battler_bird.tscn）を指定してください
    String monster_data_path = "";
    String my_model_path = "res://scenes/battler_bird.tscn"; // ←仮のパスを入れる
    int my_hp = 100;
    int my_speed = 10;

    if (party.size() > 0)
    {
        Ref<MonsterData> m = party[0];
        if (m.is_valid())
        {
            // get_path() ではなく保持しておいたパスを送る
            monster_data_path = m->get_resource_path(); 
            my_model_path = m->get_model_path();
            my_hp = m->get_max_hp();
            my_speed = m->get_speed();
            
            // ログでパスを確認
            UtilityFunctions::print("Loaded Party Monster Path: ", my_model_path);
        }
    }
    else
    {
        UtilityFunctions::print("Warning: Party is empty! Using default model.");
    }

    gm->rpc("_rpc_register_battle_ready", my_id, monster_data_path, my_model_path, my_hp, my_speed);
    if (get_tree()->get_multiplayer()->is_server())
    {
        // 自分のデータ登録RPCは一瞬遅れることがあるので、
        // わずかに待ってからチェックさせるのが最も安全です
        gm->call_deferred("_check_and_start_battle");
    }
}

void BattleScene::_rpc_start_spawning()
{
    // ここではボタンの有効化などのUI準備だけに留める
    has_selected = false;
    if (skill_button_1) skill_button_1->set_disabled(false);
    if (skill_button_2) skill_button_2->set_disabled(false);
    if (skill_button_3) skill_button_3->set_disabled(false);
    
    UtilityFunctions::print("Waiting for final battle setup from host...");
}

void BattleScene::_rpc_register_player_data(int peer_id, const String& model_path, int hp, int speed)
{
    if (!get_tree()->get_multiplayer()->is_server()) return;

    UtilityFunctions::print("Received Data from ID: ", peer_id, " Model: ", model_path);

    // データを辞書に詰める
    Dictionary info;
    info["id"] = peer_id;
    info["path"] = model_path;
    info["hp"] = hp;
    info["speed"] = speed;

    if (peer_id == 1) // ホスト自身のデータ
    {
        p1_data = info;
    }
    else // クライアントのデータ（今回は1vs1前提）
    {
        p2_data = info;
    }

    // 両方のデータが揃ったら、全員にセットアップ命令を出す
    if (!p1_data.is_empty() && !p2_data.is_empty())
    {
        UtilityFunctions::print("Both players ready! Setting up battle...");
        rpc("_rpc_setup_battle", p1_data, p2_data);
    }
}

void BattleScene::_rpc_setup_battle(const Dictionary& host_info, const Dictionary& client_info)
{
    int my_id = get_tree()->get_multiplayer()->get_unique_id();
    GameManager* gm = GameManager::get_singleton();

    // どっちが「自分」で、どっちが「敵」か判定する
    Dictionary my_data;
    Dictionary enemy_data;

    bool am_i_host = (my_id == 1);

    if (am_i_host)
    {
        // 私はホスト
        my_data = host_info;
        enemy_data = client_info;
    }
    else
    {
        // 私はクライアント
        my_data = client_info;
        enemy_data = host_info;
    }

    // --- HPなどの設定 ---
    player_hp = (int)my_data["hp"];
    enemy_hp = (int)enemy_data["hp"];
    
    // GameManagerにも敵情報を入れておく（ダメージ計算などで使う）
    if (gm)
    {
        gm->set_player_current_hp(player_hp);
        gm->set_next_enemy_stats("Opponent", enemy_hp, 5); // 攻撃力なども送るべきですが簡易設定
        gm->set_next_enemy_speed((int)enemy_data["speed"]);
    }

    // --- モデルのスポーン ---
    
    // 1. 自分のモデルを PlayerSpawnPos に出す
    _spawn_model_at((String)my_data["path"], player_spawn_pos, true);

    // 2. 敵のモデルを EnemySpawnPos に出す
    _spawn_model_at((String)enemy_data["path"], enemy_spawn_pos, false);

    // ボタン有効化
    has_selected = false;
    
    // スキルボタンの更新（自分の持っているスキルで更新）
    if (gm && gm->get_party().size() > 0)
    {
        Ref<MonsterData> leader = gm->get_party()[0];
        if (leader.is_valid())
        {
            current_skills = leader->get_skills();
            _update_ui_buttons();
        }
    }

    battle_ready = true;
    UtilityFunctions::print("Battle Setup Complete! My HP: ", player_hp, " Enemy HP: ", enemy_hp);
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
    int my_id = get_tree()->get_multiplayer()->get_unique_id();
    GameManager* gm = GameManager::get_singleton();
    if (!gm) return;

    // --- 1. 3Dモデルの取得 ---
    Node3D* player_model = nullptr;
    Node3D* enemy_model = nullptr;

    if (player_spawn_pos && player_spawn_pos->get_child_count() > 0)
        player_model = Object::cast_to<Node3D>(player_spawn_pos->get_child(0));
    if (enemy_spawn_pos && enemy_spawn_pos->get_child_count() > 0)
        enemy_model = Object::cast_to<Node3D>(enemy_spawn_pos->get_child(0));

    // --- 2. ステータスと計算 ---
    int my_atk = gm->get_player_attack();
    int my_def = gm->get_player_defense();
    int enemy_atk = gm->get_next_enemy_attack();
    int enemy_def = gm->get_next_enemy_defense();

    int damage_to_enemy = MAX(1, my_atk - enemy_def);
    int damage_to_player = MAX(1, enemy_atk - my_def);

    // --- 3. スキルリストの取得 ---
    TypedArray<SkillData> my_skills;
    if (gm->get_party().size() > 0) {
        Ref<MonsterData> m = gm->get_party()[0];
        if (m.is_valid()) my_skills = m->get_skills();
    }
    // 相手のスキルリスト（前回修正した変数を使用）
    TypedArray<SkillData> current_enemy_skills = enemy_player_skills;

    String my_hand_str = is_server ? h_hand : c_hand;
    String enemy_hand_str = is_server ? c_hand : h_hand;

    // ★重要: アタッカーとディフェンダーを決定するロジック
    Node3D* attacker_node = nullptr;
    Node3D* defender_node = nullptr;
    Ref<SkillData> skill_to_use;
    bool is_player_attacking = false; // 自分が攻撃側かどうか

    // 勝敗判定
    if (winner_side != 0) // 決着がついた
    {
        // winner_side: 1=Hostが勝った, 2=Clientが勝った
        bool host_won = (winner_side == 1);
        
        // 「自分が勝った」かどうかの判定
        bool i_won = (is_server && host_won) || (!is_server && !host_won);

        if (i_won)
        {
            // 自分が勝った -> 自分のモデルがアタッカー
            is_player_attacking = true;
            attacker_node = player_model;
            defender_node = enemy_model;
            skill_to_use = _get_skill_by_hand(my_skills, my_hand_str);
            
            // ダメージ適用（データ上）
            enemy_hp -= damage_to_enemy;
            UtilityFunctions::print("I WON! Attacking enemy.");
        }
        else
        {
            // 相手が勝った -> 相手のモデルがアタッカー
            is_player_attacking = false;
            attacker_node = enemy_model;
            defender_node = player_model;
            
            // 相手の手に対応するスキルを取得
            // (注意: 相手の手は enemy_hand_str に入っています)
            skill_to_use = _get_skill_by_hand(current_enemy_skills, enemy_hand_str);
            
            // ダメージ適用
            player_hp -= damage_to_player;
            UtilityFunctions::print("I LOST! Enemy is attacking me.");
        }
    }
    else 
    {
        // あいこ (一旦省略しますが、同様に attacker_node を設定する必要があります)
        UtilityFunctions::print("Draw match!");
        // あいこの場合は一旦処理をスキップ、または先制攻撃ロジックを入れる
    }

    // --- 4. 演出実行 ---
    if (attacker_node && defender_node && skill_to_use.is_valid())
    {
        _perform_attack_sequence(attacker_node, defender_node, skill_to_use);
    }
    else if (attacker_node && defender_node)
    {
        // スキルが見つからない場合の通常攻撃（フォールバック）
        UtilityFunctions::print("Warning: Skill not found, but performing move sequence.");
        // 仮のスキルデータを作るか、移動だけさせる
    }

    // --- 5. ゲーム終了判定 (変更なし) ---
    // Update UI calls here...

    if (player_hp <= 0){
        player_hp = 0;
        if (is_server) rpc("_rpc_notify_defeat");
        UtilityFunctions::print("DEFEATED...");
        get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
    }
    else if (enemy_hp <= 0) {
        enemy_hp = 0;
        UtilityFunctions::print("VICTORY!");
        if(gm) gm->gain_experience(gm->get_next_enemy_exp_reward());
        get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
    }
    else {
        has_selected = false;
        _update_ui_buttons();
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

Ref<SkillData> BattleScene::_get_skill_by_hand(const TypedArray<SkillData>& skills, const String& hand_str)
{
    int type = -1;
    if (hand_str == "rock") type = 0;
    else if (hand_str == "scissors") type = 1;
    else if (hand_str == "paper") type = 2;

    for (int i = 0; i < skills.size(); ++i)
    {
        Ref<SkillData> skill = skills[i];
        if (skill.is_valid() && skill->get_hand_type() == type)
        {
            return skill;
        }
    }
    return nullptr;
}

String resolve_anim_name(AnimationPlayer* anim, String name)
{
    if (name.is_empty())
    {
        return "";
    }
    String prefixed = "CharacterArmature|" + name;
    if (anim->has_animation(prefixed))
    {
        return prefixed;
    }
    return name;
}

void BattleScene::_perform_attack_sequence(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill)
{
    if (!attacker || !target || skill.is_null())
    {
        return;
    }

    AnimationPlayer* anim = Object::cast_to<AnimationPlayer>(attacker->find_child("AnimationPlayer", true, false));
    if (!anim) 
    {
        return;
    }

    // 以前の Tween が残っていると誤作動するため、新しく作成
    Ref<Tween> tween = create_tween();
    if (tween.is_null())
    {
        return;
    }

    String skill_anim = resolve_anim_name(anim, skill->get_animation_name());
    String idle_anim = resolve_anim_name(anim, "Idle");
    String jump_anim = resolve_anim_name(anim, "Jump");

    Vector3 start_pos = attacker->get_global_position();
    Vector3 target_pos = target->get_global_position();
    Vector3 dir = (target_pos - start_pos).normalized();

    String skill_name = skill->get_skill_name();

    if (skill_name == "たいあたり")
    {
        // --- たいあたり演出 ---
        Vector3 attack_pos = target_pos - (dir * 0.4);

        // 1. 高速突進（0.15秒で移動）
        tween->tween_property(attacker, "global_position", attack_pos, 0.15)->set_trans(Tween::TRANS_SINE);
        
        // 2. 突進と同時にアニメーション開始
        tween->parallel()->tween_callback(Callable(anim, "play").bind(skill_anim));

        // 3. ヒットの余韻（移動が終わった後、0.4秒止まる）
        tween->tween_interval(0.4);

        // 4. 元の位置へ戻る（0.3秒）
        tween->tween_property(attacker, "global_position", start_pos, 0.3);
        tween->tween_callback(Callable(anim, "play").bind(idle_anim));
    }
    else if (skill_name == "のしかかる")
{
    // --- のしかかる演出 ---
    // 頂点を少し高め（3.5mなど）に設定すると放物線が綺麗に見えます
    Vector3 peak_pos = (start_pos + target_pos) / 2.0 + Vector3(0, 3.5, 0);

    // 1. ジャンプ上昇：徐々に遅くなる（EASE_OUT）
    tween->tween_callback(Callable(anim, "play").bind(jump_anim));
    tween->tween_property(attacker, "global_position", peak_pos, 0.4)
        ->set_trans(Tween::TRANS_QUAD)
        ->set_ease(Tween::EASE_OUT);

    // 2. 落下：徐々に速くなる（EASE_IN）
    tween->tween_callback(Callable(anim, "play").bind(skill_anim));
    tween->tween_property(attacker, "global_position", target_pos, 0.3)
        ->set_trans(Tween::TRANS_QUAD)
        ->set_ease(Tween::EASE_IN);

    // 3. ヒット時の停止
    tween->tween_interval(0.5);

    // 4. 元の位置へ戻る（ここも放物線にするとより自然です）
    Vector3 return_peak = (target_pos + start_pos) / 2.0 + Vector3(0, 2.0, 0);
    tween->tween_property(attacker, "global_position", return_peak, 0.25)
        ->set_trans(Tween::TRANS_QUAD)
        ->set_ease(Tween::EASE_OUT);
    tween->tween_property(attacker, "global_position", start_pos, 0.2)
        ->set_trans(Tween::TRANS_QUAD)
        ->set_ease(Tween::EASE_IN);
    
    tween->tween_callback(Callable(anim, "play").bind(idle_anim));
}
    else
    {
        // --- デフォルト：かみつく等の物理移動 ---
        Vector3 attack_pos = target_pos - (dir * 1.2); 

        tween->tween_property(attacker, "global_position", attack_pos, 0.2);
        tween->parallel()->tween_callback(Callable(anim, "play").bind(skill_anim));
        
        tween->tween_interval(0.7);
        
        tween->tween_property(attacker, "global_position", start_pos, 0.2);
        tween->tween_callback(Callable(anim, "play").bind(idle_anim));
    }
}

void BattleScene::_spawn_model_at(const String& path, Node3D* parent_node, bool is_player)
{
    if (!parent_node || path.is_empty())
    {
        return;
    }

    for (int i = 0; i < parent_node->get_child_count(); ++i)
    {
        parent_node->get_child(i)->queue_free();
    }

    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
    if (scene.is_valid())
    {
        Node* instance = scene->instantiate();
        Node3D* model_3d = Object::cast_to<Node3D>(instance);
        if (model_3d)
        {
            if (is_player)
            {
                model_3d->set_rotation_degrees(Vector3(0, 180, 0));
            }
            parent_node->add_child(model_3d);
            model_3d->set_position(Vector3(0, 0, 0)); 

            AnimationPlayer* anim = Object::cast_to<AnimationPlayer>(model_3d->find_child("AnimationPlayer", true, false));
            if (anim)
            {
                // --- 自動プレフィックス判定 ---
                String base_name = "Idle";
                String prefixed_name = "CharacterArmature|" + base_name;

                if (anim->has_animation(prefixed_name))
                {
                    anim->play(prefixed_name);
                }
                else
                {
                    anim->play(base_name); // プレフィックスなしを試す
                }
            }
        }
    }
}

void BattleScene::request_battle_data()
{
    GameManager* gm = GameManager::get_singleton();
    if (!gm) return;

    if (get_tree()->get_multiplayer()->is_server())
    {
        // ホスト自身なら直接チェック
        gm->_check_and_start_battle();
    }
    else
    {
        // クライアントならサーバーに「データ送信」を依頼する
        // GameManagerにリクエスト用のRPCを作るか、単に自身のロード完了を通知する
        rpc_id(1, "_rpc_notify_loaded"); 
    }
}