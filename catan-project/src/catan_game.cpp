#include "catan_game.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void CatanGame::_bind_methods()
{
    // メソッドのバインド（ここではRPCの設定は行わず、公開のみ行う）
    ClassDB::bind_method(D_METHOD("host_game", "port"), &CatanGame::host_game, DEFVAL(53000));
    ClassDB::bind_method(D_METHOD("join_game", "address", "port"), &CatanGame::join_game, DEFVAL("127.0.0.1"), DEFVAL(53000));
    
    ClassDB::bind_method(D_METHOD("request_roll_dice"), &CatanGame::request_roll_dice);
    ClassDB::bind_method(D_METHOD("notify_dice_result", "roll_value"), &CatanGame::notify_dice_result);
    ClassDB::bind_method(D_METHOD("start_game"), &CatanGame::start_game);
    ClassDB::bind_method(D_METHOD("rpc_change_scene", "scene_path"), &CatanGame::rpc_change_scene);
    ADD_SIGNAL(MethodInfo("dice_rolled", PropertyInfo(Variant::INT, "roll_value")));

    ClassDB::bind_method(D_METHOD("request_build_settlement", "vertex_name"), &CatanGame::request_build_settlement);
    ClassDB::bind_method(D_METHOD("server_process_build", "vertex_name"), &CatanGame::server_process_build);
    ClassDB::bind_method(D_METHOD("client_sync_build", "vertex_name", "player_id"), &CatanGame::client_sync_build);
    
    // 画面(GDScript)に「家が建ったよ！」と知らせるシグナル
    ADD_SIGNAL(MethodInfo("settlement_built", PropertyInfo(Variant::STRING, "vertex_name"), PropertyInfo(Variant::INT, "player_id")));

    ClassDB::bind_method(D_METHOD("request_build_road", "edge_name"), &CatanGame::request_build_road);
    ClassDB::bind_method(D_METHOD("server_process_build_road", "edge_name"), &CatanGame::server_process_build_road);
    ClassDB::bind_method(D_METHOD("client_sync_build_road", "edge_name", "player_id"), &CatanGame::client_sync_build_road);
    
    // 画面に「道が建ったよ」と知らせるシグナル
    ADD_SIGNAL(MethodInfo("road_built", PropertyInfo(Variant::STRING, "edge_name"), PropertyInfo(Variant::INT, "player_id")));

    ClassDB::bind_method(D_METHOD("register_vertex", "vertex_name", "pos"), &CatanGame::register_vertex);

    ClassDB::bind_method(D_METHOD("add_resource", "player_id", "resource_type", "amount"), &CatanGame::add_resource);
    ClassDB::bind_method(D_METHOD("client_sync_resources", "player_id", "wood", "brick", "sheep", "wheat", "ore"), &CatanGame::client_sync_resources);
    
    // GDScriptへ「UI（画面）を更新して！」と伝えるシグナル
    ADD_SIGNAL(MethodInfo("resources_updated", 
        PropertyInfo(Variant::INT, "player_id"), 
        PropertyInfo(Variant::INT, "wood"), 
        PropertyInfo(Variant::INT, "brick"), 
        PropertyInfo(Variant::INT, "sheep"), 
        PropertyInfo(Variant::INT, "wheat"), 
        PropertyInfo(Variant::INT, "ore")));

    ClassDB::bind_method(D_METHOD("distribute_resources_for_hex", "hex_center", "hex_radius", "resource_type"), &CatanGame::distribute_resources_for_hex);

    ClassDB::bind_method(D_METHOD("register_edge", "edge_name", "midpoint"), &CatanGame::register_edge);

    ClassDB::bind_method(D_METHOD("start_turn_system"), &CatanGame::start_turn_system);
    ClassDB::bind_method(D_METHOD("request_end_turn"), &CatanGame::request_end_turn);
    ClassDB::bind_method(D_METHOD("server_process_end_turn"), &CatanGame::server_process_end_turn);
    ClassDB::bind_method(D_METHOD("client_sync_turn", "player_id"), &CatanGame::client_sync_turn);

    // 画面(GDScript)に「ターンが切り替わったよ」と知らせるシグナル
    ADD_SIGNAL(MethodInfo("turn_changed", PropertyInfo(Variant::INT, "player_id")));
}

CatanGame::CatanGame()
{
    // ネットワークピアのインスタンス化
    peer.instantiate();

    // --- RPC設定の修正 ---
    // インスタンスメソッドの rpc_config をコンストラクタ内で使用します。
    // 第1引数はメソッド名（StringName）、第2引数はDictionary形式の設定です。

    // 1. 画面切り替え (サーバーから全員へ、自分も実行)
    Dictionary change_scene_conf;
    change_scene_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    change_scene_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    change_scene_conf["call_local"] = true;
    change_scene_conf["channel"] = 0;
    rpc_config("rpc_change_scene", change_scene_conf);

    // 2. サイコロの結果通知 (サーバーから全員へ、自分も実行)
    Dictionary notify_dice_conf;
    notify_dice_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    notify_dice_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    notify_dice_conf["call_local"] = true;
    notify_dice_conf["channel"] = 0;
    rpc_config("notify_dice_result", notify_dice_conf);

    // 3. サイコロ振ってリクエスト (クライアント誰からでもサーバーへ)
    Dictionary req_roll_conf;
    req_roll_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER; // 誰でも呼んでいいよ
    req_roll_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    req_roll_conf["call_local"] = true; // 自分自身（サーバー）が押した場合も実行
    req_roll_conf["channel"] = 0;
    rpc_config("request_roll_dice", req_roll_conf);

    Dictionary server_build_conf;
    server_build_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    server_build_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    server_build_conf["call_local"] = true;
    server_build_conf["channel"] = 0;
    rpc_config("server_process_build", server_build_conf);

    // サーバーから全員へ送る設定 (AUTHORITY)
    Dictionary client_build_conf;
    client_build_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    client_build_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    client_build_conf["call_local"] = true;
    client_build_conf["channel"] = 0;
    rpc_config("client_sync_build", client_build_conf);

    Dictionary server_road_conf;
    server_road_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    server_road_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    server_road_conf["call_local"] = true;
    server_road_conf["channel"] = 0;
    rpc_config("server_process_build_road", server_road_conf);

    Dictionary client_road_conf;
    client_road_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    client_road_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    client_road_conf["call_local"] = true;
    client_road_conf["channel"] = 0;
    rpc_config("client_sync_build_road", client_road_conf);

    Dictionary sync_res_conf;
    sync_res_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY; // サーバーからのみ送信
    sync_res_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    sync_res_conf["call_local"] = true;
    sync_res_conf["channel"] = 0;
    rpc_config("client_sync_resources", sync_res_conf);

    // --- ターン管理用のRPC設定 ---
    Dictionary end_turn_conf;
    end_turn_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    end_turn_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    end_turn_conf["call_local"] = true;
    end_turn_conf["channel"] = 0;
    rpc_config("server_process_end_turn", end_turn_conf);

    Dictionary sync_turn_conf;
    sync_turn_conf["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    sync_turn_conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    sync_turn_conf["call_local"] = true;
    sync_turn_conf["channel"] = 0;
    rpc_config("client_sync_turn", sync_turn_conf);
}

CatanGame::~CatanGame()
{
}

void CatanGame::host_game(int port)
{
    peer->create_server(port);
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);
    UtilityFunctions::print("Server started on port ", port);
}

void CatanGame::join_game(const String& address, int port)
{
    peer->create_client(address, port);
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);
    UtilityFunctions::print("Connecting to ", address, ":", port);
}

void CatanGame::request_roll_dice()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    int dice_roll = (rand() % 6 + 1) + (rand() % 6 + 1);
    rpc("notify_dice_result", dice_roll);
}

void CatanGame::notify_dice_result(int roll_value)
{
    UtilityFunctions::print("Client: Dice rolled! Result: ", roll_value);
    emit_signal("dice_rolled", roll_value);
}

void CatanGame::start_game() {
    if (get_tree()->get_multiplayer()->is_server()) {
        // call_local を true に設定すると、サーバー自身もこのRPCを実行します
        rpc("rpc_change_scene", "res://scenes/Main.tscn");
    }
}

void CatanGame::rpc_change_scene(const String& scene_path) {
    UtilityFunctions::print("Changing scene to: ", scene_path);
    // 全員のGodotエンジンに「シーンを切り替えろ」と命令
    get_tree()->change_scene_to_file(scene_path);
}

// 1. GDScriptから呼ばれる（自分からサーバーへお願いする）
void CatanGame::request_build_settlement(const String& vertex_name) {
    // サーバー(ID: 1)に対して、RPCを実行
    rpc_id(1, "server_process_build", vertex_name);
}

// 2. サーバーだけが受け取って実行する
// 家を建てる処理をパワーアップ！
void CatanGame::server_process_build(const String& vertex_name) {
    if (!get_tree()->get_multiplayer()->is_server()) return;
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0) sender_id = 1;

    // ★ 1. コストチェック（開拓地＝木1,土1,羊1,麦1）
    PlayerData& p = players[sender_id];
    if (p.wood < 1 || p.brick < 1 || p.sheep < 1 || p.wheat < 1) {
        UtilityFunctions::print("Server: 資源が足りないため家を建てられません！");
        return;
    }

    // すでに家がないかチェック
    if (board_vertices[vertex_name].owner_id != 0) return; 

    // 距離ルールのチェック（既存のまま）
    Vector2 my_pos = board_vertices[vertex_name].position;
    for (const auto& pair : board_vertices) {
        if (pair.second.owner_id != 0) {
            float dist = my_pos.distance_to(pair.second.position);
            if (dist > 0.1f && dist < 80.0f) return; // 近すぎる
        }
    }

    // ★ 2. コスト消費とUI更新！
    p.wood -= 1;
    p.brick -= 1;
    p.sheep -= 1;
    p.wheat -= 1;
    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);

    board_vertices[vertex_name].owner_id = sender_id;
    board_vertices[vertex_name].building_type = 1;

    rpc("client_sync_build", vertex_name, sender_id);
}

// 3. 全員が受け取って画面を更新する
void CatanGame::client_sync_build(const String& vertex_name, int player_id) {
    UtilityFunctions::print("Player ", player_id, " built at ", vertex_name);
    // GDScript側にシグナルを飛ばして、見た目を更新させる
    emit_signal("settlement_built", vertex_name, player_id);
}

void CatanGame::request_build_road(const String& edge_name) {
    rpc_id(1, "server_process_build_road", edge_name);
}

void CatanGame::server_process_build_road(const String& edge_name) {
    if (!get_tree()->get_multiplayer()->is_server()) return;
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0) sender_id = 1;

    // ★ 1. コストチェック（街道＝木1,土1）
    PlayerData& p = players[sender_id];
    if (p.wood < 1 || p.brick < 1) {
        UtilityFunctions::print("Server: 資源が足りないため道を引けません！");
        return;
    }

    if (board_edges[edge_name].owner_id != 0) return; 

    // ★ 2. 接続ルール（自分の家、または自分の道に繋がっているか？）のチェック
    bool is_connected = false;
    Vector2 edge_pos = board_edges[edge_name].midpoint;

    for (const auto& v_pair : board_vertices) {
        // 辺の中心から約25px（余裕を持って35未満）にある頂点が、この辺の両端の交差点
        if (edge_pos.distance_to(v_pair.second.position) < 35.0f) {
            
            // パターンA: その交差点に自分の家がある
            if (v_pair.second.owner_id == sender_id) {
                is_connected = true;
                break;
            }
            
            // パターンB: その交差点に繋がる「他の自分の道」がある
            for (const auto& e_pair : board_edges) {
                if (e_pair.first == edge_name) continue; // 自分自身は無視
                
                if (e_pair.second.owner_id == sender_id) {
                    // その道も同じ交差点に繋がっているなら（頂点から距離35未満）
                    if (v_pair.second.position.distance_to(e_pair.second.midpoint) < 35.0f) {
                        is_connected = true;
                        break;
                    }
                }
            }
            if (is_connected) break;
        }
    }

    if (!is_connected) {
        UtilityFunctions::print("Server: 自分の家か道に繋がっていない場所には引けません！");
        return;
    }

    // ★ 3. コスト消費とUI更新！
    p.wood -= 1;
    p.brick -= 1;
    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);

    board_edges[edge_name].owner_id = sender_id;
    rpc("client_sync_build_road", edge_name, sender_id);
}

void CatanGame::client_sync_build_road(const String& edge_name, int player_id) {
    emit_signal("road_built", edge_name, player_id);
}

// 座標をサーバーの記憶（map）に保存する関数
void CatanGame::register_vertex(const String& vertex_name, Vector2 pos) {
    board_vertices[vertex_name].position = pos;
}

// サーバーがプレイヤーに資源を与える関数
void CatanGame::add_resource(int player_id, const String& resource_type, int amount) {
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // 指定された資源を増やす
    if (resource_type == "wood") players[player_id].wood += amount;
    else if (resource_type == "brick") players[player_id].brick += amount;
    else if (resource_type == "sheep") players[player_id].sheep += amount;
    else if (resource_type == "wheat") players[player_id].wheat += amount;
    else if (resource_type == "ore") players[player_id].ore += amount;

    // 最新の資源量を全員に送信して同期する
    PlayerData& p = players[player_id];
    rpc("client_sync_resources", player_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
}

// 全員が受け取って画面のUIを更新する関数
void CatanGame::client_sync_resources(int player_id, int wood, int brick, int sheep, int wheat, int ore) {
    // GDScript側にシグナルを飛ばす
    emit_signal("resources_updated", player_id, wood, brick, sheep, wheat, ore);
}

void CatanGame::distribute_resources_for_hex(Vector2 hex_center, float hex_radius, const String& resource_type) {
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // 砂漠などは資源を配らないので無視
    if (resource_type == "desert" || resource_type == "none" || resource_type == "") return;

    // すべての交差点を1つずつ確認する
    for (const auto& pair : board_vertices) {
        // もしそこに誰かの家が建っていたら
        if (pair.second.owner_id != 0) {
            
            // タイルの中心から、その家までの距離を測る
            float dist = pair.second.position.distance_to(hex_center);
            
            // 距離がタイルの半径とほぼ同じなら（誤差5pxを許容）、そのタイルにくっついている！
            if (dist > (hex_radius - 5.0f) && dist < (hex_radius + 5.0f)) {
                
                int amount = pair.second.building_type; // 家なら1個、都市なら2個
                UtilityFunctions::print("Server: Hex ", resource_type, " gives ", amount, " to Player ", pair.second.owner_id);
                
                // 持ち主に資源を追加！
                add_resource(pair.second.owner_id, resource_type, amount);
            }
        }
    }
}

void CatanGame::register_edge(const String& edge_name, Vector2 midpoint) {
    board_edges[edge_name].midpoint = midpoint;
}

// サーバーがプレイヤー全員のリストを作って最初のターンを開始する
void CatanGame::start_turn_system() {
    if (!get_tree()->get_multiplayer()->is_server()) return;

    player_order.clear();
    player_order.push_back(1); // ホスト(ID:1)を最初に追加

    // 接続している他のプレイヤー(クライアント)を取得して追加
    PackedInt32Array peers = get_tree()->get_multiplayer()->get_peers();
    for (int i = 0; i < peers.size(); i++) {
        player_order.push_back(peers[i]);
    }

    current_turn_index = 0;
    
    // 全員に最初のプレイヤーのターンであることを通知！
    rpc("client_sync_turn", player_order[current_turn_index]);
}

void CatanGame::request_end_turn() {
    rpc_id(1, "server_process_end_turn");
}

void CatanGame::server_process_end_turn() {
    if (!get_tree()->get_multiplayer()->is_server()) return;
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0) sender_id = 1;

    // もし自分のターンじゃないのに「終了」しようとしたら弾く（チート対策）
    if (player_order.size() > 0 && sender_id != player_order[current_turn_index]) {
        UtilityFunctions::print("Server: あなたのターンではありません！");
        return;
    }

    // 次の人のインデックスへ（最後まで行ったら0に戻る）
    current_turn_index = (current_turn_index + 1) % player_order.size();
    
    // 全員に「次の人のターンになったよ」と通知！
    rpc("client_sync_turn", player_order[current_turn_index]);
}

void CatanGame::client_sync_turn(int player_id) {
    UtilityFunctions::print("Turn changed to Player: ", player_id);
    emit_signal("turn_changed", player_id);
}