#include "catan_game.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

CatanGame::CatanGame()
{
    // ネットワークピアのインスタンス化
    peer.instantiate();

    // --- RPC設定の修正 ---
    // インスタンスメソッドの rpc_config をコンストラクタ内で使用します。
    // 第1引数はメソッド名（StringName）、第2引数はDictionary形式の設定です。

    // 1. request_roll_dice: クライアントからサーバーへ (Any Peer -> Authority)
    Dictionary rpc_config_request;
    rpc_config_request["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
    rpc_config_request["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    rpc_config_request["call_local"] = false;
    
    rpc_config("request_roll_dice", rpc_config_request);

    // 2. notify_dice_result: サーバーから全員へ (Authority -> All)
    Dictionary rpc_config_notify;
    rpc_config_notify["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
    rpc_config_notify["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
    rpc_config_notify["call_local"] = true;
    
    rpc_config("notify_dice_result", rpc_config_notify);
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
}

void CatanGame::_bind_methods()
{
    // メソッドのバインド（ここではRPCの設定は行わず、公開のみ行う）
    ClassDB::bind_method(D_METHOD("host_game", "port"), &CatanGame::host_game, DEFVAL(53000));
    ClassDB::bind_method(D_METHOD("join_game", "address", "port"), &CatanGame::join_game, DEFVAL("127.0.0.1"), DEFVAL(53000));
    
    ClassDB::bind_method(D_METHOD("request_roll_dice"), &CatanGame::request_roll_dice);
    ClassDB::bind_method(D_METHOD("notify_dice_result", "roll_value"), &CatanGame::notify_dice_result);
}