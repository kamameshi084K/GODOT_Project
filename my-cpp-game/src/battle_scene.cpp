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
#include <godot_cpp/classes/style_box_flat.hpp>

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

    ClassDB::bind_method(D_METHOD("_rpc_sync_hp", "target_side", "final_damage"), &BattleScene::_rpc_sync_hp);
    ClassDB::bind_method(D_METHOD("_apply_damage", "attacker", "target", "skill"), &BattleScene::_apply_damage);
    ClassDB::bind_method(D_METHOD("_show_janken_ui", "p_hand", "e_hand"), &BattleScene::_show_janken_ui);
    ClassDB::bind_method(D_METHOD("_hide_janken_ui"), &BattleScene::_hide_janken_ui);
    
    // ★ 変更: 引数を減らしました
    ClassDB::bind_method(D_METHOD("_perform_attack_sequence", "attacker", "target", "skill"), &BattleScene::_perform_attack_sequence);
    
    // ★ 追加: ターン終了処理用
    ClassDB::bind_method(D_METHOD("_on_draw_or_end"), &BattleScene::_on_draw_or_end);

    ClassDB::bind_method(D_METHOD("show_message", "text"), &BattleScene::show_message);
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
    rpc_config("_rpc_sync_hp", rpc_config_auth);
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
    player_hp_bar = get_node<ProgressBar>("UI/PlayerHPBar");
    enemy_hp_bar = get_node<ProgressBar>("UI/EnemyHPBar");
    message_label = get_node<Label>("UI/MessageWindow/Label");

    // _ready() の中に追加
    janken_effect_root = get_node<Control>("UI/JankenEffect");
    left_hand_rect = get_node<TextureRect>("UI/JankenEffect/PlayerHand");
    right_hand_rect = get_node<TextureRect>("UI/JankenEffect/EnemyHand");
    if (!janken_effect_root) UtilityFunctions::print("Error: JankenEffect node not found!");
    if (!left_hand_rect) UtilityFunctions::print("Error: PlayerHand node not found!");

    // 画像のロード（パスは実際のプロジェクトに合わせて変更してください）
    tex_rock = ResourceLoader::get_singleton()->load("res://assets/textures/rock.png");
    tex_scissors = ResourceLoader::get_singleton()->load("res://assets/textures/scissors.png");
    tex_paper = ResourceLoader::get_singleton()->load("res://assets/textures/paper.png");

    // 【追加】ロード確認ログ
    if (tex_rock.is_null()) UtilityFunctions::print("Error: Failed to load rock.png");
    if (tex_scissors.is_null()) UtilityFunctions::print("Error: Failed to load scissors.png");
    if (tex_paper.is_null()) UtilityFunctions::print("Error: Failed to load paper.png");

    // 初期状態ではメッセージを隠しておくなどの処理
    if (message_label)
    {
        Control* parent_ui = Object::cast_to<Control>(message_label->get_parent());
        if (parent_ui)
        {
            parent_ui->set_visible(false);
        }
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

    GameManager* gm = GameManager::get_singleton();
    if (!gm || !get_tree() || !get_tree()->get_multiplayer().is_valid()) return;

    int my_id = get_tree()->get_multiplayer()->get_unique_id();
    TypedArray<MonsterData> party = gm->get_party();
    
    String my_model_path = "res://scenes/battler_bird.tscn";
    int my_hp = 100;
    int my_speed = 10;
    Array skill_info_array; // スキル情報を送るための配列

    if (party.size() > 0)
    {
        Ref<MonsterData> m = party[0];
        if (m.is_valid())
        {
            my_model_path = m->get_model_path();
            my_hp = m->get_max_hp();
            my_speed = m->get_speed();

            // スキルの「名前・手・アニメ名・物理か」を辞書にして送る
            TypedArray<SkillData> skills = m->get_skills();
            for (int i = 0; i < skills.size(); ++i)
            {
                Ref<SkillData> s = skills[i];
                if (s.is_valid())
                {
                    Dictionary s_data;
                    s_data["name"] = s->get_skill_name();
                    s_data["hand"] = s->get_hand_type();
                    s_data["anim"] = s->get_animation_name();
                    s_data["phys"] = s->get_is_physical();
                    skill_info_array.append(s_data);
                }
            }
        }
    }

    // GameManager経由でサーバーに登録
    gm->rpc("_rpc_register_battle_ready", my_id, "", my_model_path, my_hp, my_speed, skill_info_array);
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

    am_i_host = (get_tree()->get_multiplayer()->get_unique_id() == 1);
    Array opponent_skill_data = am_i_host ? client_info["skills"] : host_info["skills"];

    // 相手のスキルリストをクリアして再構築
    enemy_player_skills.clear();
    for (int i = 0; i < opponent_skill_data.size(); ++i)
    {
        Dictionary s_dict = opponent_skill_data[i];
        Ref<SkillData> s_res;
        s_res.instantiate(); // ダミーのリソースを作成して情報を詰める
        s_res->set_skill_name(s_dict["name"]);
        s_res->set_hand_type(s_dict["hand"]);
        s_res->set_animation_name(s_dict["anim"]);
        s_res->set_is_physical(s_dict["phys"]);
        enemy_player_skills.append(s_res);
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
    _update_hp_bar_look(player_hp_bar, player_hp, player_hp);
    _update_hp_bar_look(enemy_hp_bar, enemy_hp, enemy_hp);
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
    String my_hand_str = is_server ? h_hand : c_hand;
    String enemy_hand_str = is_server ? c_hand : h_hand;

    // 1. 画像をセット（初期位置は画面外）
    _show_janken_ui(my_hand_str, enemy_hand_str);

    // --- 勝敗判定をここで計算しておく（Tweenで使うため） ---
    bool i_won = false;
    bool enemy_won = false;
    
    if (winner_side != 0) {
        bool host_won = (winner_side == 1);
        i_won = (is_server && host_won) || (!is_server && !host_won);
        enemy_won = !i_won;
    }
    // 両方falseなら引き分け

    // 2. 攻撃データの準備 (計算処理・変更なし)
    Node3D* player_model = nullptr;
    Node3D* enemy_model = nullptr;
    if (player_spawn_pos && player_spawn_pos->get_child_count() > 0)
        player_model = Object::cast_to<Node3D>(player_spawn_pos->get_child(0));
    if (enemy_spawn_pos && enemy_spawn_pos->get_child_count() > 0)
        enemy_model = Object::cast_to<Node3D>(enemy_spawn_pos->get_child(0));

    Node3D* attacker_node = nullptr;
    Node3D* defender_node = nullptr;
    Ref<SkillData> skill_to_use;
    
    if (winner_side != 0) {
        if (i_won) {
            attacker_node = player_model;
            defender_node = enemy_model;
            skill_to_use = _get_skill_by_hand(current_skills, my_hand_str);
        } else {
            attacker_node = enemy_model;
            defender_node = player_model;
            skill_to_use = _get_skill_by_hand(enemy_player_skills, enemy_hand_str);
        }
    }

    // --- 3. Tweenでアニメーション演出 ---
    Ref<Tween> seq = create_tween();
    if (seq.is_null()) return;

    // 演出用の座標計算 (スケール0.2に合わせて調整)
    Vector2 view_size = janken_effect_root ? janken_effect_root->get_size() : Vector2(1152, 648);
    float center_x = view_size.x / 2.0;
    
    // スケール定義
    float base_scale = 0.2;
    Vector2 tex_size = left_hand_rect->get_size(); // _show_janken_uiでセット済みと仮定
    float scaled_width = tex_size.x * base_scale;
    
    // 中央で競り合う位置（中心から少しだけオフセットしてぶつかる感じに）
    // 完全に中央だと重なるので、半径分だけずらす
    float offset_from_center = scaled_width * 0.5; 
    
    // Pivotが中心(size/2)にある前提でのLeft/RightのRect座標(TopLeft)計算
    // VisualCenter = Pos + Pivot
    // Pos = VisualCenter - Pivot
    // Left Visual Center Goal = center_x - offset_from_center
    // Right Visual Center Goal = center_x + offset_from_center
    
    float left_target_x = (center_x - offset_from_center) - (tex_size.x / 2.0);
    float right_target_x = (center_x + offset_from_center) - (tex_size.x / 2.0);

    // (A) 「画面外から中央へ」ガッと移動 (0.3秒)
    seq->set_parallel(true); // 並列実行開始
    seq->tween_property(left_hand_rect, "position:x", left_target_x, 0.3)->set_trans(Tween::TRANS_CUBIC)->set_ease(Tween::EASE_OUT);
    seq->tween_property(right_hand_rect, "position:x", right_target_x, 0.3)->set_trans(Tween::TRANS_CUBIC)->set_ease(Tween::EASE_OUT);
    
    // (B) 衝突の衝撃で少し揺らすなど（省略可だが、間を作る）
    seq->chain()->tween_interval(0.2); // 移動完了後、0.2秒溜める

    // (C) 勝敗演出：勝ったほうを前に、負けたほうを暗く (0.3秒かけて変化)
    seq->set_parallel(true);
    
    // 勝った時の拡大サイズ (0.2 -> 0.3)
    Vector2 win_scale = Vector2(0.3, 0.3);

    if (winner_side == 0) {
        // 引き分け：両方ちょっと弾かれる動きなど（今回はそのまま）
    } else {
        if (i_won) {
            // 自分勝ち：自分を強調、相手を暗く
            seq->tween_property(left_hand_rect, "scale", win_scale, 0.3)->set_trans(Tween::TRANS_BACK)->set_ease(Tween::EASE_OUT);
            seq->tween_property(right_hand_rect, "modulate", Color(0.3, 0.3, 0.3, 1), 0.3); // 暗くする
        } else {
            // 相手勝ち：相手を強調、自分を暗く
            seq->tween_property(right_hand_rect, "scale", win_scale, 0.3)->set_trans(Tween::TRANS_BACK)->set_ease(Tween::EASE_OUT);
            seq->tween_property(left_hand_rect, "modulate", Color(0.3, 0.3, 0.3, 1), 0.3); // 暗くする
        }
    }

    // (D) 結果をじっくり見せる時間 (0.8秒)
    seq->chain()->tween_interval(0.8);

    // (E) 手を隠す
    seq->tween_callback(Callable(this, "_hide_janken_ui"));

    // (F) 攻撃へ移行 or ターン終了
    if (attacker_node && defender_node && skill_to_use.is_valid())
    {
        seq->tween_callback(Callable(this, "_perform_attack_sequence").bind(attacker_node, defender_node, skill_to_use));
        seq->tween_interval(2.0); 
        seq->tween_callback(Callable(this, "_on_draw_or_end"));
    }
    else
    {
        seq->tween_interval(0.5);
        seq->tween_callback(Callable(this, "_on_draw_or_end")); 
    }
}

void BattleScene::_rpc_notify_defeat()
{
    UtilityFunctions::print("VICTORY! Returning to town...");
    get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
}

void BattleScene::_update_ui_buttons()
{
    // 色の定義（お好みで調整してください）
    Color color_rock = Color(1.0, 0.4, 0.4, 1.0);    // 赤 (グー)
    Color color_scissors = Color(1.0, 0.9, 0.2, 1.0); // 黄 (チョキ)
    Color color_paper = Color(0.4, 0.6, 1.0, 1.0);    // 青 (パー)
    Color color_disable = Color(0.5, 0.5, 0.5, 1.0);  // グレー (無効時)

    auto update_btn = [&](Button* btn, int index)
    {
        if (!btn) return;

        if (current_skills.size() > index)
        {
            Ref<SkillData> skill = current_skills[index];
            if (skill.is_valid())
            {
                int type = skill->get_hand_type();
                String type_str = _hand_type_to_string(type);
                btn->set_text(skill->get_skill_name() + "\n(" + type_str + ")");
                btn->set_disabled(false);

                // ★ 手の種類に応じてボタンの色を変える
                switch (type)
                {
                    case 0: btn->set_modulate(color_rock); break;     // Rock
                    case 1: btn->set_modulate(color_scissors); break; // Scissors
                    case 2: btn->set_modulate(color_paper); break;    // Paper
                    default: btn->set_modulate(Color(1, 1, 1)); break;
                }
            }
        }
        else
        {
            btn->set_text("- Empty -");
            btn->set_disabled(true);
            btn->set_modulate(color_disable); // 空っぽならグレー
        }
    };

    update_btn(skill_button_1, 0);
    update_btn(skill_button_2, 1);
    update_btn(skill_button_3, 2);
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

void BattleScene::_show_janken_ui(String p_hand, String e_hand)
{
    if (janken_effect_root) {
        janken_effect_root->set_z_index(100);
        janken_effect_root->set_modulate(Color(1, 1, 1, 1));
        janken_effect_root->show();
    }

    // 画面サイズを取得（親コントロールのサイズを基準にする）
    Vector2 view_size = janken_effect_root ? janken_effect_root->get_size() : Vector2(1152, 648);

    auto set_tex = [this, view_size](TextureRect* rect, String hand, bool is_left)
    {
        if (!rect) return;
        
        // ★毎回リセット（前のターンの色が残らないように）
        rect->set_modulate(Color(1, 1, 1, 1)); 

        Ref<Texture2D> target_tex; 
        if (hand == "rock") target_tex = tex_rock;
        else if (hand == "scissors") target_tex = tex_scissors;
        else if (hand == "paper") target_tex = tex_paper;

        if (target_tex.is_valid()) {
            rect->set_texture(target_tex);
            
            // サイズとピボットを中心に設定
            Vector2 tex_size = target_tex->get_size();
            rect->set_size(tex_size);
            rect->set_pivot_offset(tex_size / 2.0);
            
            // ★初期位置の設定：画面の「外」に飛ばす
            float y_pos = (view_size.y - tex_size.y) / 2.0; // 上下中央
            if (is_left) {
                // 左手は画面左外へ (-300)
                rect->set_position(Vector2(-300, y_pos));
            } else {
                // 右手は画面右外へ (画面幅 + 300)
                rect->set_position(Vector2(view_size.x + 300, y_pos));
            }
        }
        // スケールは0.2倍
        rect->set_scale(Vector2(0.2, 0.2));
    };

    set_tex(left_hand_rect, p_hand, true);
    set_tex(right_hand_rect, e_hand, false);
}

void BattleScene::_hide_janken_ui()
{
    if (janken_effect_root) janken_effect_root->hide();
}

void BattleScene::_perform_attack_sequence(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill)
{
    // 1. 基本チェック
    if (!attacker || !target || !skill.is_valid()) return;

    // 2. アニメーター取得
    AnimationPlayer* anim = Object::cast_to<AnimationPlayer>(attacker->find_child("AnimationPlayer", true, false));
    AnimationPlayer* target_anim = Object::cast_to<AnimationPlayer>(target->find_child("AnimationPlayer", true, false));
    
    if (!anim) return;

    // 3. Tween作成
    Ref<Tween> tween = create_tween();
    if (tween.is_null()) return;

    // 4. アニメーション名の解決
    String skill_anim = resolve_anim_name(anim, skill->get_animation_name());
    String idle_anim = resolve_anim_name(anim, "Idle");
    String jump_anim = resolve_anim_name(anim, "Jump");
    String hit_anim = (target_anim) ? resolve_anim_name(target_anim, "HitRecieve") : "";

    // ★★★ 削除: ここにあった _show_janken_ui の呼び出しを消去しました ★★★
    // 純粋に攻撃モーションから開始します

    // 6. 攻撃パラメータの計算
    Vector3 start_pos = attacker->get_global_position();
    Vector3 target_pos = target->get_global_position();
    Vector3 dir = (target_pos - start_pos).normalized();
    String skill_name = skill->get_skill_name();

    // 7. スキルごとの挙動 (中身は変更なし)
    if (skill_name == String::utf8("たいあたり"))
    {
        Vector3 attack_pos = target_pos - (dir * 0.4);
        tween->tween_property(attacker, "global_position", attack_pos, 0.15)->set_trans(Tween::TRANS_SINE);
        tween->parallel()->tween_callback(Callable(anim, "play").bind(skill_anim));
        tween->tween_callback(Callable(this, "_apply_damage").bind(attacker, target, skill));
        
        if (target_anim && !hit_anim.is_empty())
            tween->tween_callback(Callable(target_anim, "play").bind(hit_anim));

        tween->tween_interval(0.5);
        tween->tween_property(attacker, "global_position", start_pos, 0.3);
        tween->tween_callback(Callable(anim, "play").bind(idle_anim));
    }
    else if (skill_name == String::utf8("のしかかる"))
    {
        Vector3 peak_pos = (start_pos + target_pos) / 2.0 + Vector3(0, 3.5, 0);
        tween->tween_callback(Callable(anim, "play").bind(jump_anim));
        tween->tween_property(attacker, "global_position", peak_pos, 0.4)->set_trans(Tween::TRANS_QUAD)->set_ease(Tween::EASE_OUT);
        tween->tween_property(attacker, "global_position", target_pos, 0.25)->set_trans(Tween::TRANS_QUAD)->set_ease(Tween::EASE_IN);
        tween->parallel()->tween_callback(Callable(anim, "play").bind(skill_anim));
        tween->tween_callback(Callable(this, "_apply_damage").bind(attacker, target, skill));

        if (target_anim && !hit_anim.is_empty())
            tween->tween_callback(Callable(target_anim, "play").bind(hit_anim));

        tween->tween_interval(0.6);
        tween->tween_property(attacker, "global_position", start_pos, 0.4);
        tween->tween_callback(Callable(anim, "play").bind(idle_anim));
    }
    else
    {
        // デフォルト攻撃
        Vector3 attack_pos = target_pos - (dir * 1.2); 
        tween->tween_property(attacker, "global_position", attack_pos, 0.2);
        tween->parallel()->tween_callback(Callable(anim, "play").bind(skill_anim));
        tween->tween_callback(Callable(this, "_apply_damage").bind(attacker, target, skill));
        
        if (target_anim && !hit_anim.is_empty())
            tween->tween_callback(Callable(target_anim, "play").bind(hit_anim));
        
        tween->tween_interval(0.7);
        tween->tween_property(attacker, "global_position", start_pos, 0.2);
        tween->tween_callback(Callable(anim, "play").bind(idle_anim));
    }

    // 8. 最後に相手をIdleに戻す
    if (target_anim)
    {
        String target_idle = resolve_anim_name(target_anim, "Idle");
        tween->tween_callback(Callable(target_anim, "play").bind(target_idle));
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

// --- _apply_damage をホスト専用の計算機にする ---
void BattleScene::_apply_damage(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill)
{
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // 攻撃者が「ホストのモデル」かどうかを判定する
    bool is_host_attacking = (attacker == player_spawn_pos->get_child(0));
    int damage = skill->get_power();

    // target_side: 1ならホストが被弾、2ならクライアントが被弾
    int target_side = is_host_attacking ? 2 : 1;

    rpc("_rpc_sync_hp", target_side, damage);
}

// --- 実質的なHP減少とUI更新を行うRPC関数 ---
void BattleScene::_rpc_sync_hp(int target_side, int final_damage)
{
    bool am_i_host = (get_tree()->get_multiplayer()->get_unique_id() == 1);
    
    // 「ホストが被弾」かつ「自分がホスト」なら自分にダメージ
    // 「クライアントが被弾」かつ「自分がクライアント」なら自分にダメージ
    bool hit_me = (target_side == 1 && am_i_host) || (target_side == 2 && !am_i_host);

    if (hit_me)
    {
        player_hp -= final_damage;
        if (player_hp < 0) player_hp = 0;
        _update_hp_bar_look(player_hp_bar, player_hp, (int)player_hp_bar->get_max());
        show_message(String::utf8("自分は ") + String::num_int64(final_damage) + String::utf8(" のダメージを受けた！"));
    }
    else
    {
        enemy_hp -= final_damage;
        if (enemy_hp < 0) enemy_hp = 0;
        _update_hp_bar_look(enemy_hp_bar, enemy_hp, (int)enemy_hp_bar->get_max());
        show_message(String::utf8("相手に ") + String::num_int64(final_damage) + String::utf8(" のダメージ！"));
    }

    // ★重要: HPが0になった瞬間の判定もここで行う
    if (player_hp <= 0 || enemy_hp <= 0)
    {
        _check_battle_end();
    }
}

void BattleScene::_check_battle_end()
{
    // すでに遷移が始まっているなら何もしない
    if (is_transitioning) return;

    if (player_hp <= 0 || enemy_hp <= 0)
    {
        is_transitioning = true; // フラグを立てる

        if (player_hp <= 0)
        {
            UtilityFunctions::print("DEFEATED...");
            get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
        }
        else
        {
            UtilityFunctions::print("VICTORY!");
            GameManager* gm = GameManager::get_singleton();
            if(gm) gm->gain_experience(gm->get_next_enemy_exp_reward());
            get_tree()->call_deferred("change_scene_to_file", "res://scenes/town.tscn");
        }
    }
}

void BattleScene::_on_draw_or_end()
{
    // 勝負が決まってシーン移動中なら何もしない
    if (is_transitioning) return;

    has_selected = false;
    _update_ui_buttons();
    
    // 必要に応じてメッセージ表示
    // show_message("コマンドを選択してください"); 
}

void BattleScene::show_message(const String& text)
{
    if (message_label)
    {
        message_label->set_text(text);
        
        // 安全に型変換して表示する
        Control* parent = Object::cast_to<Control>(message_label->get_parent());
        if (parent)
        {
            parent->show();
        }
    }
}

void BattleScene::_update_hp_bar_look(ProgressBar* bar, int current, int max)
{
    if (!bar) return;

    // 1. 値の更新
    bar->set_max(max);
    bar->set_value(current);

    // 2. 割合計算 (0.0 〜 1.0)
    float ratio = (max > 0) ? (float)current / (float)max : 0.0f;

    // 3. 色の決定 (HSV: 緑 -> 黄 -> 赤)
    float hue = ratio * 0.33f; 
    Color target_color = Color::from_hsv(hue, 1.0, 1.0);

    // 4. スタイルボックスの取得と適用
    Ref<StyleBoxFlat> flat_style;

    // すでにオーバーライド（個別設定）があるか確認
    if (bar->has_theme_stylebox_override("fill"))
    {
        Ref<StyleBox> style = bar->get_theme_stylebox("fill");
        if (style.is_valid())
        {
            // ★エラー修正箇所: 明示的に Ref<StyleBoxFlat>(...) で囲むことで曖昧さを回避
            flat_style = Ref<StyleBoxFlat>(Object::cast_to<StyleBoxFlat>(style.ptr()));
        }
    }

    // まだ設定されていない、またはStyleBoxFlat以外だった場合は新規作成
    if (flat_style.is_null())
    {
        flat_style.instantiate();
        // 角を少し丸くすると見栄えが良いです（お好みで）
        flat_style->set_corner_radius_all(4);
        bar->add_theme_stylebox_override("fill", flat_style);
    }

    // 背景色を変更
    flat_style->set_bg_color(target_color);
}