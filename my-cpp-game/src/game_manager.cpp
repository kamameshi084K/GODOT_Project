#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>


using namespace godot;

// 静的変数の初期化
GameManager *GameManager::singleton = nullptr;

void GameManager::_bind_methods()
{
    // ネットワーク関連メソッドのバインド
    ClassDB::bind_method(D_METHOD("host_game", "port"), &GameManager::host_game, DEFVAL(8910));
    ClassDB::bind_method(D_METHOD("join_game", "address", "port"), &GameManager::join_game, DEFVAL(8910));
    // メソッドのバインド
    ClassDB::bind_method(D_METHOD("set_last_player_position", "pos"), &GameManager::set_last_player_position);
    ClassDB::bind_method(D_METHOD("get_last_player_position"), &GameManager::get_last_player_position);
    
    // --- is_returning_from_battle のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("set_is_returning_from_battle", "value"), &GameManager::set_is_returning_from_battle);
    ClassDB::bind_method(D_METHOD("get_is_returning_from_battle"), &GameManager::get_is_returning_from_battle);

    // --- next_enemy_scene_path のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("set_current_enemy_id", "id"), &GameManager::set_current_enemy_id);
    ClassDB::bind_method(D_METHOD("get_current_enemy_id"), &GameManager::get_current_enemy_id);
    ClassDB::bind_method(D_METHOD("set_next_enemy_exp_reward", "value"), &GameManager::set_next_enemy_exp_reward);
    ClassDB::bind_method(D_METHOD("get_next_enemy_exp_reward"), &GameManager::get_next_enemy_exp_reward);
    
    // --- defeated_enemies のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("add_defeated_enemy", "id"), &GameManager::add_defeated_enemy);
    ClassDB::bind_method(D_METHOD("is_enemy_defeated", "id"), &GameManager::is_enemy_defeated);

    // --- ステータス関連 ---
    ClassDB::bind_method(D_METHOD("get_player_attack"), &GameManager::get_player_attack);
    ClassDB::bind_method(D_METHOD("get_player_defense"), &GameManager::get_player_defense);

    ClassDB::bind_method(D_METHOD("get_player_speed"), &GameManager::get_player_speed);
    ClassDB::bind_method(D_METHOD("get_next_enemy_speed"), &GameManager::get_next_enemy_speed);
    ClassDB::bind_method(D_METHOD("set_next_enemy_speed", "spd"), &GameManager::set_next_enemy_speed);

    ClassDB::bind_method(D_METHOD("set_next_enemy_data", "data"), &GameManager::set_next_enemy_data);
    ClassDB::bind_method(D_METHOD("get_next_enemy_data"), &GameManager::get_next_enemy_data);

    // バトルUIなどがHPバーを表示するために使う
    ClassDB::bind_method(D_METHOD("get_player_max_hp"), &GameManager::get_player_max_hp);
    ClassDB::bind_method(D_METHOD("get_player_current_hp"), &GameManager::get_player_current_hp);
    ClassDB::bind_method(D_METHOD("set_player_current_hp", "hp"), &GameManager::set_player_current_hp);

    // レベル表示用
    ClassDB::bind_method(D_METHOD("get_player_level"), &GameManager::get_player_level);
    ClassDB::bind_method(D_METHOD("get_player_exp"), &GameManager::get_player_exp);
    ClassDB::bind_method(D_METHOD("get_player_next_exp"), &GameManager::get_player_next_exp);
    
    // 経験値獲得
    ClassDB::bind_method(D_METHOD("gain_experience", "amount"), &GameManager::gain_experience);

    // 初期化用
    ClassDB::bind_method(D_METHOD("init_player_stats", "max_hp", "attack", "defense", "level", "exp", "next_exp"), &GameManager::init_player_stats);

    // セットアップ用
    ClassDB::bind_method(D_METHOD("set_last_scene_path", "path"), &GameManager::set_last_scene_path);
    // ゲット用
    ClassDB::bind_method(D_METHOD("get_last_scene_path"), &GameManager::get_last_scene_path);

    // ▼▼▼ 新機能: モンスター管理用メソッド ▼▼▼
    ClassDB::bind_method(D_METHOD("add_monster", "monster"), &GameManager::add_monster);
    ClassDB::bind_method(D_METHOD("select_starter_monster", "type_index"), &GameManager::select_starter_monster);
    ClassDB::bind_method(D_METHOD("prepare_battle_stats"), &GameManager::prepare_battle_stats);
    
    ClassDB::bind_method(D_METHOD("set_party", "party"), &GameManager::set_party);
    ClassDB::bind_method(D_METHOD("get_party"), &GameManager::get_party);
    ClassDB::bind_method(D_METHOD("get_standby"), &GameManager::get_standby);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "party_monsters", PROPERTY_HINT_RESOURCE_TYPE, "24/17:MonsterData"), "set_party", "get_party");

    ClassDB::bind_method(D_METHOD("set_starter_option_1", "data"), &GameManager::set_starter_option_1);
    ClassDB::bind_method(D_METHOD("get_starter_option_1"), &GameManager::get_starter_option_1);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "starter_option_1", PROPERTY_HINT_RESOURCE_TYPE, "MonsterData"), "set_starter_option_1", "get_starter_option_1");

    ClassDB::bind_method(D_METHOD("set_starter_option_2", "data"), &GameManager::set_starter_option_2);
    ClassDB::bind_method(D_METHOD("get_starter_option_2"), &GameManager::get_starter_option_2);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "starter_option_2", PROPERTY_HINT_RESOURCE_TYPE, "MonsterData"), "set_starter_option_2", "get_starter_option_2");

    ClassDB::bind_method(D_METHOD("set_starter_option_3", "data"), &GameManager::set_starter_option_3);
    ClassDB::bind_method(D_METHOD("get_starter_option_3"), &GameManager::get_starter_option_3);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "starter_option_3", PROPERTY_HINT_RESOURCE_TYPE, "MonsterData"), "set_starter_option_3", "get_starter_option_3");

    // 互換性維持
    ClassDB::bind_method(D_METHOD("add_collected_monster", "monster"), &GameManager::add_collected_monster);
    ClassDB::bind_method(D_METHOD("get_collected_monsters"), &GameManager::get_collected_monsters);

    // --- ゲーム進行用メソッドの登録 ---
    
    // _processを有効にするにはこれを忘れない
    // (Nodeクラスのメソッドなので通常は自動ですが、明示的にバインドは不要でもオーバーライドは必要)

    ClassDB::bind_method(D_METHOD("get_time_remaining"), &GameManager::get_time_remaining);
    ClassDB::bind_method(D_METHOD("start_collection_phase"), &GameManager::start_collection_phase);
    ClassDB::bind_method(D_METHOD("set_player_ready"), &GameManager::set_player_ready);

    // ▼▼▼ RPC（通信用関数）の登録 ▼▼▼
    // "call_local" = 自分でも実行する + 相手にも実行させる
    
    ClassDB::bind_method(D_METHOD("_rpc_start_collection"), &GameManager::_rpc_start_collection);
    ClassDB::bind_method(D_METHOD("_rpc_sync_timer", "time"), &GameManager::_rpc_sync_timer);
    ClassDB::bind_method(D_METHOD("_rpc_go_to_town"), &GameManager::_rpc_go_to_town);
    ClassDB::bind_method(D_METHOD("_rpc_notify_ready"), &GameManager::_rpc_notify_ready);
    ClassDB::bind_method(D_METHOD("_rpc_start_battle"), &GameManager::_rpc_start_battle);
    ClassDB::bind_method(D_METHOD("_rpc_register_battle_ready", "peer_id", "monster_data_path", "model_path", "hp", "speed"), &GameManager::_rpc_register_battle_ready);
    ClassDB::bind_method(D_METHOD("_check_and_start_battle"), &GameManager::_check_and_start_battle);
}

GameManager::GameManager()
{
    singleton = this;
    is_returning_from_battle = false;
    current_enemy_id = ""; // 初期化
    
    // 敵の初期値
    next_enemy_name = "Enemy";
    next_enemy_max_hp = 3;
    next_enemy_attack = 1;
    next_enemy_defense = 0;
    next_enemy_speed = 1;
    next_enemy_exp_reward = 10; // デフォルト値

    // プレイヤーの初期値
    player_max_hp = 10;
    player_current_hp = 10;
    player_attack = 5;
    player_defense = 0;
    player_speed = 5;
    player_level = 1;
    player_exp = 0;
    player_next_exp = 50;

    current_state = STATE_LOBBY;
    time_remaining = 0.0f;
    is_timer_active = false;
    ready_player_count = 0;

    p1_data.clear();
    p2_data.clear();

    // 1. _rpc_start_collection
    Dictionary rpc_config_collection;
    rpc_config_collection["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    // ★修正箇所: MultiplayerAPI ではなく MultiplayerPeer を使う
    rpc_config_collection["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    rpc_config_collection["call_local"] = true;
    rpc_config("_rpc_start_collection", rpc_config_collection); // ★この行で適用！

    // 2. _rpc_sync_timer
    Dictionary rpc_config_timer;
    rpc_config_timer["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    rpc_config_timer["call_local"] = true;
    rpc_config("_rpc_sync_timer", rpc_config_timer);

    // 3. _rpc_go_to_town
    Dictionary rpc_config_town;
    rpc_config_town["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    rpc_config_town["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE; // 移動命令は確実に届ける
    rpc_config_town["call_local"] = true;
    rpc_config("_rpc_go_to_town", rpc_config_town);

    // 4. _rpc_notify_ready
    Dictionary rpc_config_ready;
    rpc_config_ready["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    rpc_config_ready["call_local"] = false;
    rpc_config("_rpc_notify_ready", rpc_config_ready);

    // 5. _rpc_start_battle
    Dictionary rpc_config_battle;
    rpc_config_battle["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    rpc_config_battle["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    rpc_config_battle["call_local"] = true;
    rpc_config("_rpc_start_battle", rpc_config_battle);

    Dictionary rpc_config_any;
    rpc_config_any["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER; // 誰からでもOK
    rpc_config_any["call_local"] = true; // 自分自身も実行する
    rpc_config_any["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE; // 確実に届ける
    rpc_config("_rpc_register_battle_ready", rpc_config_any);
}

GameManager::~GameManager()
{
    if (singleton == this)
    {
        singleton = nullptr;
    }
}

GameManager *GameManager::get_singleton()
{
    return singleton;
}


void GameManager::host_game(int port)
{
    peer.instantiate();
    // サーバーを作成（最大4人まで）
    Error err = peer->create_server(port, 4);
    
    if (err != OK)
    {
        UtilityFunctions::print("Failed to create server! Error: ", err);
        return;
    }

    UtilityFunctions::print("Server Created! Listening on port: ", port);
    
    // GodotのマルチプレイヤーシステムにこのPeerをセット
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);
}

void GameManager::join_game(const String& address, int port)
{
    peer.instantiate();
    // クライアントとしてサーバーに参加
    Error err = peer->create_client(address, port);
    
    if (err != OK)
    {
        UtilityFunctions::print("Failed to create client! Error: ", err);
        return;
    }

    UtilityFunctions::print("Joining server at: ", address, ":", port);
    
    // GodotのマルチプレイヤーシステムにこのPeerをセット
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);
}

void GameManager::add_monster(const Ref<MonsterData>& monster)
{
    if (monster.is_null())
    {
        return;
    }

    // パーティが3体未満ならパーティに追加
    if (party_monsters.size() < 3)
    {
        party_monsters.append(monster);
        UtilityFunctions::print("Added to Party: ", monster->get_monster_name());
    }
    // 3体以上ならスタンバイ（倉庫）に追加
    else
    {
        standby_monsters.append(monster);
        UtilityFunctions::print("Party full! Sent to Standby: ", monster->get_monster_name());
    }
}

void GameManager::prepare_battle_stats()
{
    // パーティの先頭のモンスターのステータスを、戦闘システムが参照する変数にコピーする
    if (party_monsters.size() > 0)
    {
        Ref<MonsterData> leader = party_monsters[0];
        if (leader.is_valid())
        {
            player_max_hp = leader->get_max_hp();
            player_current_hp = leader->get_max_hp(); 
            
            player_attack = leader->get_attack();
            player_defense = leader->get_defense();
            player_speed = leader->get_speed(); 
            
            UtilityFunctions::print("Battle Ready! Active Monster: ", leader->get_monster_name());
        }
    }
    else
    {
        // 万が一モンスターがいない場合の安全策
        player_max_hp = 10;
        player_current_hp = 10;
        player_attack = 1;
        player_defense = 0;
    }
}

void GameManager::set_party(const TypedArray<MonsterData>& p_party)
{
    party_monsters = p_party;
}

TypedArray<MonsterData> GameManager::get_party() const
{
    return party_monsters;
}

TypedArray<MonsterData> GameManager::get_standby() const
{
    return standby_monsters;
}

void GameManager::init_player_stats(int max_hp, int attack, int defense, int level, int exp, int next_exp)
{
    player_max_hp = max_hp;
    player_current_hp = max_hp; // ゲーム開始時（またはロード時）は回復させるか、現在HPを別途保存するかによりますが、一旦Maxと同じにします
    player_attack = attack;
    player_defense = defense;
    player_level = level;
    player_exp = exp;
    player_next_exp = next_exp;
    
    UtilityFunctions::print("GameManager: Player Stats Initialized. Level: ", player_level);
}

void GameManager::set_next_enemy_stats(const String &name, int hp, int attack)
{
    next_enemy_name = name;
    next_enemy_max_hp = hp;
    next_enemy_attack = attack;
}

void GameManager::set_player_stats(int attack, int defense)
{
    player_attack = attack;
    player_defense = defense;
}

void GameManager::set_last_player_position(const Vector3 &pos)
{
    last_player_position = pos;
}

Vector3 GameManager::get_last_player_position() const
{
    return last_player_position;
}

void GameManager::set_is_returning_from_battle(bool value)
{
    is_returning_from_battle = value;
}

bool GameManager::get_is_returning_from_battle() const
{
    return is_returning_from_battle;
}

void GameManager::set_next_enemy_scene_path(const String &path)
{
    next_enemy_scene_path = path;
}

String GameManager::get_next_enemy_scene_path() const
{ 
    return next_enemy_scene_path;
}

void GameManager::set_current_enemy_id(const String &id)
{
    current_enemy_id = id;
}

String GameManager::get_current_enemy_id() const
{
    return current_enemy_id;
}

void GameManager::add_defeated_enemy(const String &id)
{
    // まだリストになければ追加する
    if (!defeated_enemies.has(id))
    {
        defeated_enemies.append(id);
    }
}

bool GameManager::is_enemy_defeated(const String &id) const
{
    return defeated_enemies.has(id);
}

String GameManager::get_next_enemy_name() const
{
    return next_enemy_name;

}
int GameManager::get_next_enemy_max_hp() const
{
    return next_enemy_max_hp;
}

int GameManager::get_next_enemy_attack() const
{
    return next_enemy_attack;
}

int GameManager::get_player_attack() const
{
    return player_attack;
}
int GameManager::get_player_defense() const
{
    return player_defense;
}

int GameManager::get_next_enemy_defense() const
{
    return next_enemy_defense;
}
void GameManager::set_next_enemy_defense(int def)
{
    next_enemy_defense = def;
}

void GameManager::set_next_enemy_exp_reward(int value)
{
    next_enemy_exp_reward = value;
}
int GameManager::get_next_enemy_exp_reward() const
{
    return next_enemy_exp_reward;
}

int GameManager::get_player_max_hp() const
{
    return player_max_hp;
}

int GameManager::get_player_current_hp() const
{
    return player_current_hp;
}

void GameManager::set_player_current_hp(int hp)
{
    player_current_hp = hp;
}

int GameManager::get_player_level() const
{
    return player_level;
}

int GameManager::get_player_exp() const
{
    return player_exp;
}

int GameManager::get_player_next_exp() const
{
    return player_next_exp;
}

void GameManager::set_last_scene_path(const String &path)
{
    last_scene_path = path;
}

String GameManager::get_last_scene_path() const
{
    return last_scene_path;
}

void GameManager::gain_experience(int amount)
{
    player_exp += amount;
    UtilityFunctions::print("Gained ", amount, " EXP. Total: ", player_exp);
    
    // ▼▼▼ レベルアップ処理 ▼▼▼
    
    // 経験値が必要量を超えている間ループする（一度に2レベル上がる場合も対応）
    while (player_exp >= player_next_exp)
    {
        player_exp -= player_next_exp; // 現在の経験値から必要分を引く
        player_level++; // レベルアップ！

        // ステータス上昇（お好みのバランスで！）
        // ※本来はMonsterData側を成長させるべきですが、簡易版として既存変数を強化
        player_max_hp += 5;
        player_attack += 2;
        player_defense += 1;
        
        // レベルアップしたら全回復させる（RPGの定番）
        player_current_hp = player_max_hp;

        // 次のレベルまでの必要経験値を増やす（1.5倍にしていくなど）
        player_next_exp = (int)(player_next_exp * 1.5); // 50 -> 75 -> 112...

        UtilityFunctions::print("LEVEL UP! Now Level: ", player_level);
        UtilityFunctions::print("HP: ", player_max_hp, " ATK: ", player_attack);
    }
}

// 互換性のためのラッパー
void GameManager::add_collected_monster(const Ref<MonsterData>& monster)
{
    add_monster(monster);
}

TypedArray<MonsterData> GameManager::get_collected_monsters() const
{
    // パーティと倉庫を合わせたリストを返す（必要に応じて）
    // 現状はパーティだけ返しておく、あるいはパーティ＋スタンバイを結合しても良い
    return party_monsters;
}

void GameManager::_process(double delta)
{
    // サーバー（ホスト）だけがタイマーを減らす
    if (is_timer_active && get_tree()->get_multiplayer()->is_server())
    {
        time_remaining -= delta;

        // タイマー情報を全員に同期（頻繁すぎると重いので、1秒に1回など間引くのが理想ですが、今は毎フレ同期はせず、UI側で推定させてもOK）
        // ここでは簡易的に「0になった瞬間」だけ確実に処理します
        
        if (time_remaining <= 0.0f)
        {
            time_remaining = 0.0f;
            is_timer_active = false;
            
            // 時間切れ！町へ移動せよ！
            rpc("_rpc_go_to_town");
        }
    }
}

// --- ゲームロジック ---

void GameManager::start_collection_phase()
{
    // ホストしか呼んではいけない
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // ゲーム開始！制限時間は 60秒（テスト用に短くしてもOK）
    time_remaining = 60.0f;
    is_timer_active = true;
    ready_player_count = 0; // リセット

    // 全員に通知
    rpc("_rpc_start_collection");
}

void GameManager::_rpc_start_collection()
{
    current_state = STATE_COLLECTION;

    time_remaining = 60.0f; // ※start_collection_phaseと同じ秒数にする
    is_timer_active = true; 

    UtilityFunctions::print("Collection Phase Started!");
    get_tree()->change_scene_to_file("res://scenes/world.tscn");
}

void GameManager::_rpc_sync_timer(float time)
{
    time_remaining = time;
}

void GameManager::_rpc_go_to_town()
{
    current_state = STATE_TOWN;
    is_timer_active = false;
    UtilityFunctions::print("Time's up! Moving to Town...");
    ready_player_count = 0;

    // 町へ移動
    get_tree()->change_scene_to_file("res://scenes/town.tscn");
}

void GameManager::set_player_ready()
{
    if (get_tree()->get_multiplayer()->is_server())
    {
        // 自分自身（サーバー）なので、RPCを使わず直接呼ぶ
        _rpc_notify_ready();
    }
    else
    {
        // クライアントなので、サーバー(ID:1)へ送る
        rpc_id(1, "_rpc_notify_ready"); 
    }
}

void GameManager::_rpc_notify_ready()
{
    // サーバーで実行される
    if (!get_tree()->get_multiplayer()->is_server()) return;

    ready_player_count++;
    UtilityFunctions::print("Player Ready! Count: ", ready_player_count);

    // 接続しているプレイヤー数を確認（簡易版：2人プレイと仮定して 2になったら開始）
    // 本来は get_tree()->get_multiplayer()->get_peers().size() + 1 (ホスト分) と比較します
    int total_players = get_tree()->get_multiplayer()->get_peers().size() + 1;

    if (ready_player_count >= total_players)
    {
        // 全員準備完了！バトル開始
        rpc("_rpc_start_battle");
    }
}

void GameManager::_rpc_start_battle()
{
    current_state = STATE_BATTLE;
    UtilityFunctions::print("Battle Start!");
    get_tree()->change_scene_to_file("res://scenes/battle.tscn");
    
    // バトルシーンへ（まだシーンがない場合はエラーになるので注意）
    // get_tree()->change_scene_to_file("res://battle.tscn"); 
    // まだバトルシーンがないならログだけ
}

float GameManager::get_time_remaining() const
{
    return time_remaining;
}

int GameManager::get_player_speed() const
{
    return player_speed;
}

int GameManager::get_next_enemy_speed() const
{
    return next_enemy_speed;
}

void GameManager::set_next_enemy_speed(int spd)
{
    next_enemy_speed = spd;
}

void GameManager::set_next_enemy_data(const Ref<MonsterData>& data)
{
    next_enemy_data = data;
    
    // 互換性のため、データがある場合は古い変数にも値を入れておくと安全です
    if (next_enemy_data.is_valid())
    {
        next_enemy_name = next_enemy_data->get_monster_name();
        next_enemy_max_hp = next_enemy_data->get_max_hp();
        next_enemy_attack = next_enemy_data->get_attack();
        next_enemy_defense = next_enemy_data->get_defense();
        next_enemy_speed = next_enemy_data->get_speed();
        
        // 経験値報酬の取得関数があればそれも（今はMonsterDataに無いかも）
        // next_enemy_exp_reward = ... 
    }
}

Ref<MonsterData> GameManager::get_next_enemy_data() const
{
    return next_enemy_data;
}

void GameManager::set_starter_option_1(const Ref<MonsterData>& data) { starter_option_1 = data; }
Ref<MonsterData> GameManager::get_starter_option_1() const { return starter_option_1; }

void GameManager::set_starter_option_2(const Ref<MonsterData>& data) { starter_option_2 = data; }
Ref<MonsterData> GameManager::get_starter_option_2() const { return starter_option_2; }

void GameManager::set_starter_option_3(const Ref<MonsterData>& data) { starter_option_3 = data; }
Ref<MonsterData> GameManager::get_starter_option_3() const { return starter_option_3; }

void GameManager::select_starter_monster(int type_index)
{
    // リセット
    party_monsters.clear();
    standby_monsters.clear();

    Ref<MonsterData> source_data;

    // 以前の switch 文を、この短いロジックに置き換えます
    switch (type_index)
    {
    case 0: source_data = starter_option_1; break; // Speed
    case 1: source_data = starter_option_2; break; // Tank
    default: source_data = starter_option_3; break; // Balance
    }

    if (source_data.is_valid())
    {
        // ★重要: duplicate(true) で複製して使う
        // これをしないと、レベルアップしたときに元のファイル(.tres)まで書き換わってしまいます
        add_monster(source_data->duplicate(true));
        
        prepare_battle_stats();
    }
    else
    {
        UtilityFunctions::print("Error: Starter monster data is not set in GameManager!");
    }
}

void GameManager::_rpc_register_battle_ready(int peer_id, const String& monster_data_path, const String& model_path, int hp, int speed)
{
    if (!get_tree()->get_multiplayer()->is_server()) return;

    Dictionary info;
    info["id"] = peer_id;
    info["monster_data_path"] = monster_data_path; // ★追加：MonsterDataのリソースパス
    info["path"] = model_path;
    info["hp"] = hp;
    info["speed"] = speed;

    if (peer_id == 1) { p1_data = info; }
    else { p2_data = info; }

    _check_and_start_battle();
}

void GameManager::_check_and_start_battle()
{
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // データが両方揃っているか確認
    if (!p1_data.is_empty() && !p2_data.is_empty())
    {
        // ルートから BattleScene を探す
        Node* battle_scene = get_tree()->get_root()->get_node_or_null("BattleScene");
        
        if (battle_scene)
        {
            UtilityFunctions::print("GameManager: All players ready. Sending setup RPC...");
            
            // ★修正: call_deferred を外して「即座に」送る
            // これにより、下の clear() が走る前に、中身が入った状態で確実に送信されます。
            battle_scene->rpc("_rpc_setup_battle", p1_data, p2_data);
            
            // 送信し終わったので、ここで初めてデータをクリア
            p1_data.clear();
            p2_data.clear();
        }
        else
        {
             UtilityFunctions::print("Waiting for BattleScene to appear...");
        }
    }
}