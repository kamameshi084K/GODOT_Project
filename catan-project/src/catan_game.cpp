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
void CatanGame::server_process_build(const String& vertex_name) {
    if (!get_tree()->get_multiplayer()->is_server()) return;

    // 誰から送られてきたかを取得
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0) sender_id = 1; // サーバー自身(Host)がクリックした場合

    // ここで本来は「すでに家がないか？」「資源は足りているか？」をチェックしますが、今回は省略！

    // 全員に「この人がここに家を建てたぞ」と命令
    rpc("client_sync_build", vertex_name, sender_id);
}

// 3. 全員が受け取って画面を更新する
void CatanGame::client_sync_build(const String& vertex_name, int player_id) {
    UtilityFunctions::print("Player ", player_id, " built at ", vertex_name);
    // GDScript側にシグナルを飛ばして、見た目を更新させる
    emit_signal("settlement_built", vertex_name, player_id);
}