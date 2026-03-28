#include "catan_game.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/array.hpp>
#include <random>

using namespace godot;

// ==========================================
// 初期化・バインド処理
// ==========================================

void CatanGame::_bind_methods()
{
    // --- 1. ネットワーク・ライフサイクル ---
    ClassDB::bind_method(D_METHOD("host_game", "port"), &CatanGame::host_game, DEFVAL(53000));
    ClassDB::bind_method(D_METHOD("join_game", "address", "port"), &CatanGame::join_game, DEFVAL("127.0.0.1"), DEFVAL(53000));
    ClassDB::bind_method(D_METHOD("start_game"), &CatanGame::start_game);
    ClassDB::bind_method(D_METHOD("rpc_change_scene", "scene_path"), &CatanGame::rpc_change_scene);
    ClassDB::bind_method(D_METHOD("register_player_name", "name"), &CatanGame::register_player_name);

    // --- 2. ターン・進行管理 ---
    ClassDB::bind_method(D_METHOD("start_turn_system"), &CatanGame::start_turn_system);
    ClassDB::bind_method(D_METHOD("request_end_turn"), &CatanGame::request_end_turn);
    ClassDB::bind_method(D_METHOD("server_process_end_turn"), &CatanGame::server_process_end_turn);
    ClassDB::bind_method(D_METHOD("client_sync_turn", "player_id", "phase"), &CatanGame::client_sync_turn);
    ClassDB::bind_method(D_METHOD("client_sync_player_list", "player_info_list"), &CatanGame::client_sync_player_list);
    
    ADD_SIGNAL(MethodInfo("player_list_updated", PropertyInfo(Variant::ARRAY, "player_info_list")));
    ADD_SIGNAL(MethodInfo("turn_changed", PropertyInfo(Variant::INT, "player_id"), PropertyInfo(Variant::INT, "phase")));

    // --- 3. ダイス・資源産出 ---
    ClassDB::bind_method(D_METHOD("request_roll_dice"), &CatanGame::request_roll_dice);
    ClassDB::bind_method(D_METHOD("notify_dice_result", "dice1", "dice2"), &CatanGame::notify_dice_result);
    ClassDB::bind_method(D_METHOD("add_resource", "player_id", "resource_type", "amount"), &CatanGame::add_resource);
    ClassDB::bind_method(D_METHOD("client_sync_resources", "player_id", "wood", "brick", "sheep", "wheat", "ore"), &CatanGame::client_sync_resources);
    ClassDB::bind_method(D_METHOD("distribute_resources_for_hex", "hex_center", "hex_radius", "resource_type"), &CatanGame::distribute_resources_for_hex);

    ADD_SIGNAL(MethodInfo("dice_rolled", PropertyInfo(Variant::INT, "dice1"), PropertyInfo(Variant::INT, "dice2")));
    ADD_SIGNAL(MethodInfo("resources_updated", PropertyInfo(Variant::INT, "player_id"), PropertyInfo(Variant::INT, "wood"), PropertyInfo(Variant::INT, "brick"), PropertyInfo(Variant::INT, "sheep"), PropertyInfo(Variant::INT, "wheat"), PropertyInfo(Variant::INT, "ore")));

    // --- 4. 建築・ボード管理 ---
    ClassDB::bind_method(D_METHOD("register_vertex", "vertex_name", "pos"), &CatanGame::register_vertex);
    ClassDB::bind_method(D_METHOD("register_edge", "edge_name", "midpoint"), &CatanGame::register_edge);
    ClassDB::bind_method(D_METHOD("register_port", "vertex_name", "port_type"), &CatanGame::register_port);

    ClassDB::bind_method(D_METHOD("request_build_settlement", "vertex_name"), &CatanGame::request_build_settlement);
    ClassDB::bind_method(D_METHOD("server_process_build", "vertex_name"), &CatanGame::server_process_build);
    ClassDB::bind_method(D_METHOD("client_sync_build", "vertex_name", "player_id"), &CatanGame::client_sync_build);
    ADD_SIGNAL(MethodInfo("settlement_built", PropertyInfo(Variant::STRING, "vertex_name"), PropertyInfo(Variant::INT, "player_id")));

    ClassDB::bind_method(D_METHOD("request_build_road", "edge_name"), &CatanGame::request_build_road);
    ClassDB::bind_method(D_METHOD("server_process_build_road", "edge_name"), &CatanGame::server_process_build_road);
    ClassDB::bind_method(D_METHOD("client_sync_build_road", "edge_name", "player_id"), &CatanGame::client_sync_build_road);
    ADD_SIGNAL(MethodInfo("road_built", PropertyInfo(Variant::STRING, "edge_name"), PropertyInfo(Variant::INT, "player_id")));

    ClassDB::bind_method(D_METHOD("request_build_city", "vertex_name"), &CatanGame::request_build_city);
    ClassDB::bind_method(D_METHOD("server_process_build_city", "vertex_name"), &CatanGame::server_process_build_city);
    ClassDB::bind_method(D_METHOD("client_sync_build_city", "vertex_name", "player_id"), &CatanGame::client_sync_build_city);
    ADD_SIGNAL(MethodInfo("city_built", PropertyInfo(Variant::STRING, "vertex_name"), PropertyInfo(Variant::INT, "player_id")));

    // --- 5. トレード（銀行・プレイヤー） ---
    ClassDB::bind_method(D_METHOD("request_bank_trade", "give_res", "get_res"), &CatanGame::request_bank_trade);
    ClassDB::bind_method(D_METHOD("server_process_bank_trade", "give_res", "get_res"), &CatanGame::server_process_bank_trade);

    ClassDB::bind_method(D_METHOD("request_propose_trade", "gw", "gb", "gs", "gwh", "go", "ww", "wb", "ws", "wwh", "wo"), &CatanGame::request_propose_trade);
    ClassDB::bind_method(D_METHOD("server_process_propose_trade", "gw", "gb", "gs", "gwh", "go", "ww", "wb", "ws", "wwh", "wo"), &CatanGame::server_process_propose_trade);
    ClassDB::bind_method(D_METHOD("client_receive_trade_proposal", "proposer_id", "gw", "gb", "gs", "gwh", "go", "ww", "wb", "ws", "wwh", "wo"), &CatanGame::client_receive_trade_proposal);
    ClassDB::bind_method(D_METHOD("request_accept_trade"), &CatanGame::request_accept_trade);
    ClassDB::bind_method(D_METHOD("server_process_accept_trade"), &CatanGame::server_process_accept_trade);
    ClassDB::bind_method(D_METHOD("client_notify_trade_accepted", "accepter_id"), &CatanGame::client_notify_trade_accepted);
    ClassDB::bind_method(D_METHOD("request_execute_trade", "target_id"), &CatanGame::request_execute_trade);
    ClassDB::bind_method(D_METHOD("server_process_execute_trade", "target_id"), &CatanGame::server_process_execute_trade);
    ClassDB::bind_method(D_METHOD("request_cancel_trade"), &CatanGame::request_cancel_trade);
    ClassDB::bind_method(D_METHOD("server_process_cancel_trade"), &CatanGame::server_process_cancel_trade);
    ClassDB::bind_method(D_METHOD("client_trade_completed"), &CatanGame::client_trade_completed);

    ADD_SIGNAL(MethodInfo("trade_proposed", PropertyInfo(Variant::INT, "proposer_id"), 
        PropertyInfo(Variant::INT, "gw"), PropertyInfo(Variant::INT, "gb"), PropertyInfo(Variant::INT, "gs"), PropertyInfo(Variant::INT, "gwh"), PropertyInfo(Variant::INT, "go"),
        PropertyInfo(Variant::INT, "ww"), PropertyInfo(Variant::INT, "wb"), PropertyInfo(Variant::INT, "ws"), PropertyInfo(Variant::INT, "wwh"), PropertyInfo(Variant::INT, "wo")));
    ADD_SIGNAL(MethodInfo("trade_accepted_by_someone", PropertyInfo(Variant::INT, "accepter_id")));
    ADD_SIGNAL(MethodInfo("trade_completed"));

    // --- 6. 発展カード ---
    ClassDB::bind_method(D_METHOD("request_buy_dev_card"), &CatanGame::request_buy_dev_card);
    ClassDB::bind_method(D_METHOD("server_process_buy_dev_card"), &CatanGame::server_process_buy_dev_card);
    ClassDB::bind_method(D_METHOD("client_sync_dev_card_bought", "player_id"), &CatanGame::client_sync_dev_card_bought);
    ClassDB::bind_method(D_METHOD("client_sync_private_dev_cards", "knight", "vp", "road", "plenty", "mono"), &CatanGame::client_sync_private_dev_cards);
    
    ADD_SIGNAL(MethodInfo("dev_card_bought", PropertyInfo(Variant::INT, "player_id")));
    ADD_SIGNAL(MethodInfo("private_dev_cards_synced", 
        PropertyInfo(Variant::INT, "knight"), 
        PropertyInfo(Variant::INT, "vp"), 
        PropertyInfo(Variant::INT, "road"), 
        PropertyInfo(Variant::INT, "plenty"), 
        PropertyInfo(Variant::INT, "mono")));

    ClassDB::bind_method(D_METHOD("request_play_knight"), &CatanGame::request_play_knight);
    ClassDB::bind_method(D_METHOD("server_process_play_knight"), &CatanGame::server_process_play_knight);
    ClassDB::bind_method(D_METHOD("client_prompt_knight_robber"), &CatanGame::client_prompt_knight_robber);
    ADD_SIGNAL(MethodInfo("prompt_knight_robber"));

    ClassDB::bind_method(D_METHOD("request_play_monopoly", "res_type"), &CatanGame::request_play_monopoly);
    ClassDB::bind_method(D_METHOD("server_process_play_monopoly", "res_type"), &CatanGame::server_process_play_monopoly);
    
    ClassDB::bind_method(D_METHOD("request_play_plenty", "res1", "res2"), &CatanGame::request_play_plenty);
    ClassDB::bind_method(D_METHOD("server_process_play_plenty", "res1", "res2"), &CatanGame::server_process_play_plenty);

    ClassDB::bind_method(D_METHOD("request_play_road_building"), &CatanGame::request_play_road_building);
    ClassDB::bind_method(D_METHOD("server_process_play_road_building"), &CatanGame::server_process_play_road_building);
    ClassDB::bind_method(D_METHOD("client_prompt_road_building"), &CatanGame::client_prompt_road_building);
    ADD_SIGNAL(MethodInfo("prompt_road_building"));

    // --- 7. 盗賊・バースト・強奪 ---
    ClassDB::bind_method(D_METHOD("set_initial_robber_pos", "pos"), &CatanGame::set_initial_robber_pos);
    ClassDB::bind_method(D_METHOD("request_move_robber", "pos"), &CatanGame::request_move_robber);
    ClassDB::bind_method(D_METHOD("server_process_move_robber", "pos"), &CatanGame::server_process_move_robber);
    ClassDB::bind_method(D_METHOD("client_sync_robber", "pos", "victims"), &CatanGame::client_sync_robber);
    ADD_SIGNAL(MethodInfo("robber_moved", PropertyInfo(Variant::VECTOR2, "pos"), PropertyInfo(Variant::ARRAY, "victims")));

    ClassDB::bind_method(D_METHOD("request_steal", "victim_id"), &CatanGame::request_steal);
    ClassDB::bind_method(D_METHOD("server_process_steal", "victim_id"), &CatanGame::server_process_steal);

    ClassDB::bind_method(D_METHOD("client_prompt_discard", "amount", "w", "b", "s", "wh", "o"), &CatanGame::client_prompt_discard);
    ClassDB::bind_method(D_METHOD("request_discard", "w", "b", "s", "wh", "o"), &CatanGame::request_discard);
    ClassDB::bind_method(D_METHOD("server_process_discard", "w", "b", "s", "wh", "o"), &CatanGame::server_process_discard);
    ClassDB::bind_method(D_METHOD("client_notify_robber_phase"), &CatanGame::client_notify_robber_phase);
    
    ADD_SIGNAL(MethodInfo("prompt_discard", 
        PropertyInfo(Variant::INT, "amount"),
        PropertyInfo(Variant::INT, "w"),
        PropertyInfo(Variant::INT, "b"),
        PropertyInfo(Variant::INT, "s"),
        PropertyInfo(Variant::INT, "wh"),
        PropertyInfo(Variant::INT, "o")));
    ADD_SIGNAL(MethodInfo("notify_robber_phase"));

    // --- 8. 称号・勝利判定 ---
    ClassDB::bind_method(D_METHOD("client_announce_winner", "winner_id"), &CatanGame::client_announce_winner);
    ClassDB::bind_method(D_METHOD("client_notify_largest_army", "player_id"), &CatanGame::client_notify_largest_army);
    ClassDB::bind_method(D_METHOD("client_notify_longest_road", "player_id"), &CatanGame::client_notify_longest_road);
    
    ADD_SIGNAL(MethodInfo("game_won", PropertyInfo(Variant::INT, "winner_id")));
    ADD_SIGNAL(MethodInfo("largest_army_changed", PropertyInfo(Variant::INT, "player_id")));
    ADD_SIGNAL(MethodInfo("longest_road_changed", PropertyInfo(Variant::INT, "player_id")));

    // --- 9. 切断・再接続 ---
    ClassDB::bind_method(D_METHOD("_on_peer_disconnected", "id"), &CatanGame::_on_peer_disconnected);
    ClassDB::bind_method(D_METHOD("_on_peer_connected", "id"), &CatanGame::_on_peer_connected);
    ClassDB::bind_method(D_METHOD("client_notify_disconnect", "player_name"), &CatanGame::client_notify_disconnect);
    ADD_SIGNAL(MethodInfo("player_disconnected", PropertyInfo(Variant::STRING, "player_name")));

    ClassDB::bind_method(D_METHOD("request_reconnect", "old_name"), &CatanGame::request_reconnect);
    ClassDB::bind_method(D_METHOD("server_process_reconnect", "old_name"), &CatanGame::server_process_reconnect);
    ClassDB::bind_method(D_METHOD("client_sync_reconnect", "old_id", "new_id", "p_name"), &CatanGame::client_sync_reconnect);
    ClassDB::bind_method(D_METHOD("client_receive_full_state", "state"), &CatanGame::client_receive_full_state);
    
    ADD_SIGNAL(MethodInfo("player_reconnected", PropertyInfo(Variant::INT, "old_id"), PropertyInfo(Variant::INT, "new_id"), PropertyInfo(Variant::STRING, "p_name")));
    ADD_SIGNAL(MethodInfo("full_state_received", PropertyInfo(Variant::DICTIONARY, "state")));
}

CatanGame::CatanGame()
{
    peer.instantiate();

    // ==========================================
    // RPC設定 (ANY_PEER -> AUTHORITY / AUTHORITY -> ALL)
    // ==========================================

    auto config_rpc = [this](const StringName& method, MultiplayerAPI::RPCMode mode, bool call_local) 
    {
        Dictionary conf;
        conf["rpc_mode"] = mode;
        conf["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE;
        conf["call_local"] = call_local;
        conf["channel"] = 0;
        this->rpc_config(method, conf);
    };

    // --- 1. ネットワーク・ライフサイクル ---
    config_rpc("rpc_change_scene", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("register_player_name", MultiplayerAPI::RPC_MODE_ANY_PEER, true);

    // --- 2. ターン・進行管理 ---
    config_rpc("server_process_end_turn", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_turn", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_sync_player_list", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 3. ダイス・資源産出 ---
    config_rpc("request_roll_dice", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("notify_dice_result", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_sync_resources", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 4. 建築・ボード管理 ---
    config_rpc("server_process_build", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_build", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_build_road", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_build_road", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_build_city", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_build_city", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 5. トレード（銀行・プレイヤー） ---
    config_rpc("server_process_bank_trade", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("server_process_propose_trade", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_receive_trade_proposal", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_accept_trade", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_notify_trade_accepted", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_execute_trade", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("server_process_cancel_trade", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_trade_completed", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 6. 発展カード ---
    config_rpc("server_process_buy_dev_card", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_dev_card_bought", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_sync_private_dev_cards", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_play_knight", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_prompt_knight_robber", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_play_monopoly", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("server_process_play_plenty", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("server_process_play_road_building", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_prompt_road_building", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 7. 盗賊・バースト・強奪 ---
    config_rpc("server_process_move_robber", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_robber", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_steal", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_prompt_discard", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_discard", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_notify_robber_phase", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 8. 称号・勝利判定 ---
    config_rpc("client_announce_winner", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_notify_largest_army", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_notify_longest_road", MultiplayerAPI::RPC_MODE_AUTHORITY, true);

    // --- 9. 切断・再接続 ---
    config_rpc("client_notify_disconnect", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("server_process_reconnect", MultiplayerAPI::RPC_MODE_ANY_PEER, true);
    config_rpc("client_sync_reconnect", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
    config_rpc("client_receive_full_state", MultiplayerAPI::RPC_MODE_AUTHORITY, true);
}

CatanGame::~CatanGame()
{
}

// ==========================================
// 1. ネットワーク・ライフサイクル管理
// ==========================================

void CatanGame::host_game(int port)
{
    peer->create_server(port);
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);

    // サーバーが接続と切断を監視
    get_tree()->get_multiplayer()->connect("peer_disconnected", Callable(this, "_on_peer_disconnected"));
    get_tree()->get_multiplayer()->connect("peer_connected", Callable(this, "_on_peer_connected"));

    UtilityFunctions::print("Server started on port ", port);
}

void CatanGame::join_game(const String& address, int port)
{
    peer->create_client(address, port);
    get_tree()->get_multiplayer()->set_multiplayer_peer(peer);
    UtilityFunctions::print("Connecting to ", address, ":", port);
}

void CatanGame::start_game()
{
    if (get_tree()->get_multiplayer()->is_server())
    {
        rpc("rpc_change_scene", "res://scenes/main.tscn");
    }
}

void CatanGame::rpc_change_scene(const String& scene_path)
{
    UtilityFunctions::print("Changing scene to: ", scene_path);
    get_tree()->change_scene_to_file(scene_path);
}

void CatanGame::register_player_name(const String& name)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    players[sender_id].player_name = name;
}

// ==========================================
// 2. ターン・進行管理
// ==========================================

void CatanGame::start_turn_system()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    initialize_dev_deck();
    player_order.clear();
    player_order.push_back(1);
    
    PackedInt32Array peers = get_tree()->get_multiplayer()->get_peers();
    for (int i = 0; i < peers.size(); i++)
    {
        player_order.push_back(peers[i]);
    }
    
    current_turn_index = 0;
    has_rolled_dice_this_turn = false;

    current_phase = PHASE_SETUP_1;
    setup_settlements_built_this_turn = 0;
    setup_roads_built_this_turn = 0;

    Array player_info_list;
    for (int i = 0; i < player_order.size(); i++)
    {
        int pid = player_order[i];
        players[pid].turn_index = i; 
        
        Dictionary info;
        info["id"] = pid;
        info["turn_index"] = players[pid].turn_index;
        info["name"] = players[pid].player_name;
        info["vp"] = 0;
        int total_hand = players[pid].wood + players[pid].brick + players[pid].sheep + players[pid].wheat + players[pid].ore;
        info["hand_count"] = total_hand;
        info["dev_cards"] = players[pid].dev_cards;
        player_info_list.push_back(info);
    }
    
    rpc("client_sync_turn", player_order[current_turn_index], current_phase); 
    rpc("client_sync_player_list", player_info_list);
}

void CatanGame::request_end_turn()
{
    rpc_id(1, "server_process_end_turn");
}

void CatanGame::server_process_end_turn()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        UtilityFunctions::print("Server: あなたのターンではありません！");
        return;
    }

    PlayerData& current_p = players[player_order[current_turn_index]];
    current_p.has_played_dev_card_this_turn = false;
    current_p.new_dev_knight = 0;
    current_p.new_dev_road = 0;
    current_p.new_dev_plenty = 0;
    current_p.new_dev_mono = 0;

    int start_index = current_turn_index;
    do
    {
        current_turn_index = (current_turn_index + 1) % player_order.size();
    } while (!players[player_order[current_turn_index]].is_connected && current_turn_index != start_index);

    has_rolled_dice_this_turn = false;
    rpc("client_sync_turn", player_order[current_turn_index], current_phase);
}

void CatanGame::client_sync_turn(int player_id, int phase)
{
    UtilityFunctions::print("Turn changed to Player: ", player_id, " Phase: ", phase);
    emit_signal("turn_changed", player_id, phase);
}

void CatanGame::client_sync_player_list(Array player_info_list)
{
    emit_signal("player_list_updated", player_info_list);
}

void CatanGame::advance_setup_turn()
{
    if (current_phase == PHASE_SETUP_1)
    {
        current_turn_index++;
        if (current_turn_index >= player_order.size())
        {
            // 1巡目終了。最後の人から逆順に2巡目開始
            current_phase = PHASE_SETUP_2;
            current_turn_index = player_order.size() - 1; 
        }
    }
    else if (current_phase == PHASE_SETUP_2)
    {
        current_turn_index--;
        if (current_turn_index < 0)
        {
            // 2巡目終了。通常フェーズへ移行し、1番目の人から開始
            current_phase = PHASE_MAIN;
            current_turn_index = 0;
            UtilityFunctions::print("Server: 初期配置完了！通常フェーズに移行します。");
        }
    }

    setup_settlements_built_this_turn = 0;
    setup_roads_built_this_turn = 0;
    has_rolled_dice_this_turn = false;

    rpc("client_sync_turn", player_order[current_turn_index], current_phase);
}

// ==========================================
// 3. ダイス・資源産出
// ==========================================

void CatanGame::request_roll_dice()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }

    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }
    if (has_rolled_dice_this_turn)
    {
        return;
    }

    has_rolled_dice_this_turn = true;

    // メルセンヌ・ツイスタによる乱数生成
    std::random_device rd;
    std::mt19937 gen(rd());
    std::uniform_int_distribution<> distrib(1, 6);

    int dice1 = distrib(gen);
    int dice2 = distrib(gen);
    int dice_roll = dice1 + dice2;

    rpc("notify_dice_result", dice1, dice2);

    if (dice_roll == 7)
    {
        server_process_roll_seven(sender_id);
    }
}

void CatanGame::notify_dice_result(int dice1, int dice2)
{
    UtilityFunctions::print("Client: Dice rolled! Result: ", dice1, " and ", dice2);
    emit_signal("dice_rolled", dice1, dice2);
}

void CatanGame::add_resource(int player_id, const String& resource_type, int amount)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }

    if (resource_type == "wood") players[player_id].wood += amount;
    else if (resource_type == "brick") players[player_id].brick += amount;
    else if (resource_type == "sheep") players[player_id].sheep += amount;
    else if (resource_type == "wheat") players[player_id].wheat += amount;
    else if (resource_type == "ore") players[player_id].ore += amount;

    PlayerData& p = players[player_id];
    rpc("client_sync_resources", player_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
}

void CatanGame::client_sync_resources(int player_id, int wood, int brick, int sheep, int wheat, int ore)
{
    emit_signal("resources_updated", player_id, wood, brick, sheep, wheat, ore);
}

void CatanGame::distribute_resources_for_hex(Vector2 hex_center, float hex_radius, const String& resource_type)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }

    if (resource_type == "desert" || resource_type == "none" || resource_type == "")
    {
        return;
    }

    if (hex_center.distance_to(robber_pos) < 5.0f)
    {
        UtilityFunctions::print("Server: 盗賊がいるため ", resource_type, " は産出されません！");
        return;
    }

    for (const auto& pair : board_vertices)
    {
        if (pair.second.owner_id != 0)
        {
            float dist = pair.second.position.distance_to(hex_center);
            
            if (dist > (hex_radius - 5.0f) && dist < (hex_radius + 5.0f))
            {
                int amount = pair.second.building_type;
                UtilityFunctions::print("Server: Hex ", resource_type, " gives ", amount, " to Player ", pair.second.owner_id);
                add_resource(pair.second.owner_id, resource_type, amount);
            }
        }
    }
}

// ==========================================
// 4. 建築・ボード管理
// ==========================================

void CatanGame::register_vertex(const String& vertex_name, Vector2 pos)
{
    board_vertices[vertex_name].position = pos;
}

void CatanGame::register_edge(const String& edge_name, Vector2 midpoint)
{
    board_edges[edge_name].midpoint = midpoint;
}

void CatanGame::register_port(const String& vertex_name, const String& port_type)
{
    if (board_vertices.count(vertex_name) > 0)
    {
        board_vertices[vertex_name].port_type = port_type;
        UtilityFunctions::print("Server: Port registered at ", vertex_name, " [Type: ", port_type, "]");
    }
}

void CatanGame::request_build_settlement(const String& vertex_name)
{
    rpc_id(1, "server_process_build", vertex_name);
}

void CatanGame::server_process_build(const String& vertex_name)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        UtilityFunctions::print("Server: あなたのターンではないため建築できません！");
        return;
    }

    PlayerData& p = players[sender_id];

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        if (setup_settlements_built_this_turn >= 1)
        {
            UtilityFunctions::print("Server: 初期配置では家を1つしか建てられません！");
            return;
        }
    }
    else
    {
        if (!has_rolled_dice_this_turn)
        {
            UtilityFunctions::print("Server: サイコロを振るまでは建築できません！");
            return;
        }
        if (p.wood < 1 || p.brick < 1 || p.sheep < 1 || p.wheat < 1)
        {
            UtilityFunctions::print("Server: 資源が足りないため家を建てられません！");
            return;
        }
    }

    if (board_vertices[vertex_name].owner_id != 0)
    {
        UtilityFunctions::print("Server: すでに建物があります！");
        return; 
    }

    Vector2 my_pos = board_vertices[vertex_name].position;
    for (const auto& pair : board_vertices)
    {
        if (pair.second.owner_id != 0)
        {
            float dist = my_pos.distance_to(pair.second.position);
            if (dist > 0.1f && dist < 80.0f)
            {
                UtilityFunctions::print("Server: 他の家と近すぎます！");
                return;
            }
        }
    }

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        setup_settlements_built_this_turn++;
    }
    else
    {
        p.wood -= 1; p.brick -= 1; p.sheep -= 1; p.wheat -= 1;
        rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
    }

    board_vertices[vertex_name].owner_id = sender_id;
    board_vertices[vertex_name].building_type = 1;

    rpc("client_sync_build", vertex_name, sender_id);

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        if (setup_settlements_built_this_turn >= 1 && setup_roads_built_this_turn >= 1)
        {
            advance_setup_turn(); 
        }
    }
    
    update_longest_road();
    server_check_victory(sender_id);
}

void CatanGame::client_sync_build(const String& vertex_name, int player_id)
{
    UtilityFunctions::print("Player ", player_id, " built at ", vertex_name);
    emit_signal("settlement_built", vertex_name, player_id);
}

void CatanGame::request_build_road(const String& edge_name)
{
    rpc_id(1, "server_process_build_road", edge_name);
}

void CatanGame::server_process_build_road(const String& edge_name)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }

    PlayerData& p = players[sender_id];
    bool using_free_road = false;

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        if (setup_roads_built_this_turn >= 1)
        {
            return;
        }
    }
    else
    {
        if (!has_rolled_dice_this_turn)
        {
            return;
        }
        
        if (p.free_roads_available > 0)
        {
            using_free_road = true;
        }
        else if (p.wood < 1 || p.brick < 1)
        {
            return;
        }
    }

    if (board_edges[edge_name].owner_id != 0)
    {
        return; 
    }

    bool is_connected = false;
    Vector2 edge_pos = board_edges[edge_name].midpoint;
    
    for (const auto& v_pair : board_vertices)
    {
        if (edge_pos.distance_to(v_pair.second.position) < 35.0f)
        {
            if (v_pair.second.owner_id == sender_id)
            {
                is_connected = true;
                break;
            }
            
            for (const auto& e_pair : board_edges)
            {
                if (e_pair.first == edge_name)
                {
                    continue;
                }
                
                if (e_pair.second.owner_id == sender_id)
                {
                    if (v_pair.second.position.distance_to(e_pair.second.midpoint) < 35.0f)
                    {
                        is_connected = true;
                        break;
                    }
                }
            }
            if (is_connected)
            {
                break;
            }
        }
    }

    if (!is_connected)
    {
        UtilityFunctions::print("Server: 繋がっていない場所です！");
        return;
    }

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        setup_roads_built_this_turn++;
    }
    else
    {
        if (using_free_road)
        {
            p.free_roads_available--;
            UtilityFunctions::print("Server: 無料で道を引きました。残り無料枠: ", p.free_roads_available);
        }
        else
        {
            p.wood -= 1; p.brick -= 1;
            rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
        }
    }

    board_edges[edge_name].owner_id = sender_id;
    rpc("client_sync_build_road", edge_name, sender_id);

    if (current_phase == PHASE_SETUP_1 || current_phase == PHASE_SETUP_2)
    {
        if (setup_settlements_built_this_turn >= 1 && setup_roads_built_this_turn >= 1)
        {
            advance_setup_turn();
        }
    }
    
    update_longest_road();
    server_check_victory(sender_id);
}

void CatanGame::client_sync_build_road(const String& edge_name, int player_id)
{
    emit_signal("road_built", edge_name, player_id);
}

void CatanGame::request_build_city(const String& vertex_name)
{
    rpc_id(1, "server_process_build_city", vertex_name);
}

void CatanGame::server_process_build_city(const String& vertex_name)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }
    if (current_phase != PHASE_MAIN)
    {
        return;
    }
    if (!has_rolled_dice_this_turn)
    {
        return;
    }

    PlayerData& p = players[sender_id];

    if (p.wheat < 2 || p.ore < 3)
    {
        UtilityFunctions::print("Server: 資源が足りないため都市化できません！");
        return;
    }

    if (board_vertices[vertex_name].owner_id != sender_id)
    {
        UtilityFunctions::print("Server: 自分の家以外の場所は都市化できません！");
        return;
    }
    if (board_vertices[vertex_name].building_type != 1)
    {
        UtilityFunctions::print("Server: すでに都市化されているか、家がありません！");
        return;
    }

    p.wheat -= 2;
    p.ore -= 3;
    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);

    board_vertices[vertex_name].building_type = 2;
    rpc("client_sync_build_city", vertex_name, sender_id);
    
    update_longest_road();
    server_check_victory(sender_id);
}

void CatanGame::client_sync_build_city(const String& vertex_name, int player_id)
{
    emit_signal("city_built", vertex_name, player_id);
}

// ==========================================
// 5. トレード（銀行・プレイヤー）
// ==========================================

void CatanGame::request_bank_trade(const String& give_res, const String& get_res)
{
    rpc_id(1, "server_process_bank_trade", give_res, get_res);
}

void CatanGame::server_process_bank_trade(const String& give_res, const String& get_res)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        UtilityFunctions::print("Server: あなたのターンではありません！");
        return;
    }
    if (current_phase != PHASE_MAIN)
    {
        UtilityFunctions::print("Server: 初期配置中はトレードできません！");
        return;
    }

    PlayerData& p = players[sender_id];
    int trade_rate = 4;
    bool has_general_port = false;
    bool has_special_port = false;

    for (const auto& pair : board_vertices)
    {
        if (pair.second.owner_id == sender_id)
        {
            String p_type = pair.second.port_type;
            if (p_type == "3:1")
            {
                has_general_port = true;
            }
            else if (p_type == give_res)
            {
                has_special_port = true;
            }
        }
    }

    if (has_special_port)
    {
        trade_rate = 2;
    }
    else if (has_general_port)
    {
        trade_rate = 3;
    }

    bool can_trade = false;
    if (give_res == "wood" && p.wood >= trade_rate) { p.wood -= trade_rate; can_trade = true; }
    else if (give_res == "brick" && p.brick >= trade_rate) { p.brick -= trade_rate; can_trade = true; }
    else if (give_res == "sheep" && p.sheep >= trade_rate) { p.sheep -= trade_rate; can_trade = true; }
    else if (give_res == "wheat" && p.wheat >= trade_rate) { p.wheat -= trade_rate; can_trade = true; }
    else if (give_res == "ore" && p.ore >= trade_rate) { p.ore -= trade_rate; can_trade = true; }

    if (!can_trade)
    {
        UtilityFunctions::print("Server: 資源が ", trade_rate, " 個足りないためトレードできません！");
        return;
    }

    if (get_res == "wood") p.wood += 1;
    else if (get_res == "brick") p.brick += 1;
    else if (get_res == "sheep") p.sheep += 1;
    else if (get_res == "wheat") p.wheat += 1;
    else if (get_res == "ore") p.ore += 1;

    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
    UtilityFunctions::print("Server: Player ", sender_id, " traded ", trade_rate, " ", give_res, " for 1 ", get_res);
}

void CatanGame::request_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo)
{
    rpc_id(1, "server_process_propose_trade", gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo);
}

void CatanGame::server_process_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }

    PlayerData& p = players[sender_id];
    if (p.wood < gw || p.brick < gb || p.sheep < gs || p.wheat < gwh || p.ore < go)
    {
        return;
    }

    is_player_trade_active = true;
    trade_proposer_id = sender_id;
    
    t_gw = gw; t_gb = gb; t_gs = gs; t_gwh = gwh; t_go = go;
    t_ww = ww; t_wb = wb; t_ws = ws; t_wwh = wwh; t_wo = wo;

    for (int pid : player_order)
    {
        rpc_id(pid, "client_receive_trade_proposal", sender_id, gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo);
    }
}

void CatanGame::client_receive_trade_proposal(int proposer_id, int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo)
{
    emit_signal("trade_proposed", proposer_id, gw, gb, gs, gwh, go, ww, wb, ws, wwh, wo);
}

void CatanGame::request_accept_trade()
{
    rpc_id(1, "server_process_accept_trade");
}

void CatanGame::server_process_accept_trade()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int accepter_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (accepter_id == 0)
    {
        accepter_id = 1;
    }

    if (!is_player_trade_active || accepter_id == trade_proposer_id)
    {
        return;
    }

    PlayerData& p_acc = players[accepter_id];
    if (p_acc.wood < t_ww || p_acc.brick < t_wb || p_acc.sheep < t_ws || p_acc.wheat < t_wwh || p_acc.ore < t_wo)
    {
        return;
    }

    rpc_id(trade_proposer_id, "client_notify_trade_accepted", accepter_id);
}

void CatanGame::client_notify_trade_accepted(int accepter_id)
{
    emit_signal("trade_accepted_by_someone", accepter_id);
}

void CatanGame::request_execute_trade(int target_id)
{
    rpc_id(1, "server_process_execute_trade", target_id);
}

void CatanGame::server_process_execute_trade(int target_id)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (!is_player_trade_active || sender_id != trade_proposer_id)
    {
        return;
    }

    PlayerData& p_prop = players[trade_proposer_id];
    PlayerData& p_acc = players[target_id];

    if (p_prop.wood < t_gw || p_prop.brick < t_gb || p_prop.sheep < t_gs || p_prop.wheat < t_gwh || p_prop.ore < t_go)
    {
        return;
    }
    
    if (p_acc.wood < t_ww || p_acc.brick < t_wb || p_acc.sheep < t_ws || p_acc.wheat < t_wwh || p_acc.ore < t_wo)
    {
        return;
    }

    p_prop.wood -= t_gw; p_prop.brick -= t_gb; p_prop.sheep -= t_gs; p_prop.wheat -= t_gwh; p_prop.ore -= t_go;
    p_acc.wood  += t_gw; p_acc.brick  += t_gb; p_acc.sheep  += t_gs; p_acc.wheat  += t_gwh; p_acc.ore  += t_go;

    p_acc.wood -= t_ww; p_acc.brick -= t_wb; p_acc.sheep -= t_ws; p_acc.wheat -= t_wwh; p_acc.ore -= t_wo;
    p_prop.wood += t_ww; p_prop.brick += t_wb; p_prop.sheep += t_ws; p_prop.wheat += t_wwh; p_prop.ore += t_wo;

    is_player_trade_active = false;
    
    rpc("client_sync_resources", trade_proposer_id, p_prop.wood, p_prop.brick, p_prop.sheep, p_prop.wheat, p_prop.ore);
    rpc("client_sync_resources", target_id, p_acc.wood, p_acc.brick, p_acc.sheep, p_acc.wheat, p_acc.ore);
    rpc("client_trade_completed");
}

void CatanGame::request_cancel_trade()
{
    rpc_id(1, "server_process_cancel_trade");
}

void CatanGame::server_process_cancel_trade()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (is_player_trade_active && sender_id == trade_proposer_id)
    {
        is_player_trade_active = false;
        rpc("client_trade_completed");
    }
}

void CatanGame::client_trade_completed()
{
    emit_signal("trade_completed");
}

// ==========================================
// 6. 発展カード
// ==========================================

void CatanGame::initialize_dev_deck()
{
    dev_card_deck.clear();
    for(int i=0; i<14; i++) dev_card_deck.push_back("knight");
    for(int i=0; i<5; i++)  dev_card_deck.push_back("vp");
    for(int i=0; i<2; i++)  dev_card_deck.push_back("road_building");
    for(int i=0; i<2; i++)  dev_card_deck.push_back("year_of_plenty");
    for(int i=0; i<2; i++)  dev_card_deck.push_back("monopoly");

    std::random_device rd;
    std::mt19937 g(rd());
    std::shuffle(dev_card_deck.begin(), dev_card_deck.end(), g);
}

void CatanGame::request_buy_dev_card()
{
    rpc_id(1, "server_process_buy_dev_card");
}

void CatanGame::server_process_buy_dev_card()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }
    if (current_phase != PHASE_MAIN)
    {
        return;
    }
    if (!has_rolled_dice_this_turn)
    {
        return;
    }

    PlayerData& p = players[sender_id];

    if (p.wheat < 1 || p.sheep < 1 || p.ore < 1)
    {
        UtilityFunctions::print("Server: 資源が足りないため発展カードを買えません！");
        return;
    }

    if (dev_card_deck.empty())
    {
        UtilityFunctions::print("Server: 発展カードの山札がもうありません！");
        return;
    }

    p.wheat -= 1; p.sheep -= 1; p.ore -= 1;
    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);

    String drawn_card = dev_card_deck.back();
    dev_card_deck.pop_back();
    p.dev_cards += 1;

    if (drawn_card == "knight") { p.dev_knight++; p.new_dev_knight++; }
    else if (drawn_card == "vp") { p.dev_vp++; }
    else if (drawn_card == "road_building") { p.dev_road++; p.new_dev_road++; }
    else if (drawn_card == "year_of_plenty") { p.dev_plenty++; p.new_dev_plenty++; }
    else if (drawn_card == "monopoly") { p.dev_mono++; p.new_dev_mono++; }

    rpc("client_sync_dev_card_bought", sender_id);
    rpc_id(sender_id, "client_sync_private_dev_cards", p.dev_knight, p.dev_vp, p.dev_road, p.dev_plenty, p.dev_mono);
    
    server_check_victory(sender_id);
}

void CatanGame::client_sync_dev_card_bought(int player_id)
{
    emit_signal("dev_card_bought", player_id);
}

void CatanGame::client_sync_private_dev_cards(int knight, int vp, int road, int plenty, int mono)
{
    emit_signal("private_dev_cards_synced", knight, vp, road, plenty, mono);
}

void CatanGame::request_play_knight()
{
    rpc_id(1, "server_process_play_knight");
}

void CatanGame::server_process_play_knight()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    if (player_order.size() > 0 && sender_id != player_order[current_turn_index])
    {
        return;
    }

    PlayerData& p = players[sender_id];
    
    if (p.dev_knight <= 0)
    {
        UtilityFunctions::print("Server: 騎士カードを持っていません！");
        return;
    }

    if (p.has_played_dev_card_this_turn)
    {
        UtilityFunctions::print("Server: 1ターンに使える発展カードは1枚だけです！");
        return;
    }
    
    if ((p.dev_knight - p.new_dev_knight) <= 0)
    {
        UtilityFunctions::print("Server: 引いたばかりのカードは次のターンまで使えません！");
        return;
    }

    p.dev_knight -= 1;
    p.dev_cards -= 1;
    p.knights_played += 1;
    
    UtilityFunctions::print("Server: Player ", sender_id, " played Knight. Total: ", p.knights_played);

    if (p.knights_played > largest_army_count)
    {
        largest_army_count = p.knights_played;
        if (largest_army_player != sender_id)
        {
            largest_army_player = sender_id;
            UtilityFunctions::print("Server: Player ", sender_id, " takes Largest Army!");
            rpc("client_notify_largest_army", sender_id);
        }
    }
    
    server_check_victory(sender_id);

    rpc_id(sender_id, "client_sync_private_dev_cards", p.dev_knight, p.dev_vp, p.dev_road, p.dev_plenty, p.dev_mono);
    rpc_id(sender_id, "client_prompt_knight_robber");
}

void CatanGame::client_prompt_knight_robber()
{
    emit_signal("prompt_knight_robber");
}

void CatanGame::request_play_monopoly(const String& res_type)
{
    rpc_id(1, "server_process_play_monopoly", res_type);
}

void CatanGame::server_process_play_monopoly(const String& res_type)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    PlayerData& p = players[sender_id];
    if (p.dev_mono <= 0)
    {
        return;
    }

    p.dev_mono -= 1;
    p.dev_cards -= 1;
    p.has_played_dev_card_this_turn = true;

    int total_stolen = 0;
    
    for (auto& pair : players)
    {
        int target_id = pair.first;
        if (target_id == sender_id)
        {
            continue;
        }
        
        PlayerData& target_p = pair.second;
        int amount = 0;
        
        if (res_type == "wood") { amount = target_p.wood; target_p.wood = 0; }
        else if (res_type == "brick") { amount = target_p.brick; target_p.brick = 0; }
        else if (res_type == "sheep") { amount = target_p.sheep; target_p.sheep = 0; }
        else if (res_type == "wheat") { amount = target_p.wheat; target_p.wheat = 0; }
        else if (res_type == "ore") { amount = target_p.ore; target_p.ore = 0; }
        
        total_stolen += amount;
        rpc("client_sync_resources", target_id, target_p.wood, target_p.brick, target_p.sheep, target_p.wheat, target_p.ore);
    }

    if (res_type == "wood") p.wood += total_stolen;
    else if (res_type == "brick") p.brick += total_stolen;
    else if (res_type == "sheep") p.sheep += total_stolen;
    else if (res_type == "wheat") p.wheat += total_stolen;
    else if (res_type == "ore") p.ore += total_stolen;

    UtilityFunctions::print("Server: Player ", sender_id, " played Monopoly on ", res_type, ". Got ", total_stolen);

    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
    rpc_id(sender_id, "client_sync_private_dev_cards", p.dev_knight, p.dev_vp, p.dev_road, p.dev_plenty, p.dev_mono);
}

void CatanGame::request_play_plenty(const String& res1, const String& res2)
{
    rpc_id(1, "server_process_play_plenty", res1, res2);
}

void CatanGame::server_process_play_plenty(const String& res1, const String& res2)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    PlayerData& p = players[sender_id];
    if (p.dev_plenty <= 0)
    {
        return;
    }

    p.dev_plenty -= 1;
    p.dev_cards -= 1;
    p.has_played_dev_card_this_turn = true;

    if (res1 == "wood") p.wood++; else if (res1 == "brick") p.brick++; else if (res1 == "sheep") p.sheep++; else if (res1 == "wheat") p.wheat++; else if (res1 == "ore") p.ore++;
    if (res2 == "wood") p.wood++; else if (res2 == "brick") p.brick++; else if (res2 == "sheep") p.sheep++; else if (res2 == "wheat") p.wheat++; else if (res2 == "ore") p.ore++;

    UtilityFunctions::print("Server: Player ", sender_id, " played Year of Plenty for ", res1, " and ", res2);

    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
    rpc_id(sender_id, "client_sync_private_dev_cards", p.dev_knight, p.dev_vp, p.dev_road, p.dev_plenty, p.dev_mono);
}

void CatanGame::request_play_road_building()
{
    rpc_id(1, "server_process_play_road_building");
}

void CatanGame::server_process_play_road_building()
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    PlayerData& p = players[sender_id];
    if (p.dev_road <= 0)
    {
        return;
    }

    p.dev_road -= 1;
    p.dev_cards -= 1;
    p.has_played_dev_card_this_turn = true;
    p.free_roads_available += 2;

    rpc_id(sender_id, "client_sync_private_dev_cards", p.dev_knight, p.dev_vp, p.dev_road, p.dev_plenty, p.dev_mono);
    rpc_id(sender_id, "client_prompt_road_building");
}

void CatanGame::client_prompt_road_building()
{
    emit_signal("prompt_road_building");
}

// ==========================================
// 7. 盗賊・バースト・強奪
// ==========================================

void CatanGame::set_initial_robber_pos(Vector2 pos)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    robber_pos = pos;
    UtilityFunctions::print("Server: 盗賊の初期位置を砂漠に設定しました ", pos);
}

void CatanGame::server_process_roll_seven(int roller_id)
{
    pending_discard_players = 0;

    for (int pid : player_order)
    {
        PlayerData& p = players[pid];
        int total_hand = p.wood + p.brick + p.sheep + p.wheat + p.ore;
        
        if (total_hand >= 7)
        {
            int amount_to_discard = total_hand / 2; 
            pending_discard_players++;
            p.is_waiting_for_discard = true;
            
            rpc_id(pid, "client_prompt_discard", amount_to_discard, p.wood, p.brick, p.sheep, p.wheat, p.ore);
        }
    }

    if (pending_discard_players == 0)
    {
        rpc_id(roller_id, "client_notify_robber_phase");
    }
}

void CatanGame::client_prompt_discard(int amount, int w, int b, int s, int wh, int o)
{
    emit_signal("prompt_discard", amount, w, b, s, wh, o);
}

void CatanGame::request_discard(int w, int b, int s, int wh, int o)
{
    rpc_id(1, "server_process_discard", w, b, s, wh, o);
}

void CatanGame::server_process_discard(int w, int b, int s, int wh, int o)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    PlayerData& p = players[sender_id];
    p.wood -= w; p.brick -= b; p.sheep -= s; p.wheat -= wh; p.ore -= o;
    p.is_waiting_for_discard = false;
    
    rpc("client_sync_resources", sender_id, p.wood, p.brick, p.sheep, p.wheat, p.ore);
    pending_discard_players--;

    if (pending_discard_players <= 0)
    {
        pending_discard_players = 0;
        int roller_id = player_order[current_turn_index];
        rpc_id(roller_id, "client_notify_robber_phase");
    }
}

void CatanGame::client_notify_robber_phase()
{
    emit_signal("notify_robber_phase");
}

void CatanGame::request_move_robber(Vector2 pos)
{
    rpc_id(1, "server_process_move_robber", pos);
}

void CatanGame::server_process_move_robber(Vector2 pos)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }
    
    if (pos.distance_to(robber_pos) < 5.0f)
    {
        UtilityFunctions::print("Server: 同じ場所には盗賊を置けません！");
        return; 
    }
    
    robber_pos = pos;
    Array victims;
    float hex_radius = 54.0f;
    
    for (const auto& pair : board_vertices)
    {
        if (pair.second.owner_id != 0 && pair.second.owner_id != sender_id)
        {
            float dist = pair.second.position.distance_to(pos);
            if (dist > (hex_radius - 5.0f) && dist < (hex_radius + 5.0f))
            {
                if (!victims.has(pair.second.owner_id))
                {
                    victims.push_back(pair.second.owner_id);
                }
            }
        }
    }
    rpc("client_sync_robber", pos, victims);
}

void CatanGame::client_sync_robber(Vector2 pos, Array victims)
{
    emit_signal("robber_moved", pos, victims);
}

void CatanGame::request_steal(int victim_id)
{
    rpc_id(1, "server_process_steal", victim_id);
}

void CatanGame::server_process_steal(int victim_id)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int sender_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (sender_id == 0)
    {
        sender_id = 1;
    }

    PlayerData& v_data = players[victim_id];
    PlayerData& s_data = players[sender_id];
    
    std::vector<String> available_resources;
    for (int i = 0; i < v_data.wood; i++) available_resources.push_back("wood");
    for (int i = 0; i < v_data.brick; i++) available_resources.push_back("brick");
    for (int i = 0; i < v_data.sheep; i++) available_resources.push_back("sheep");
    for (int i = 0; i < v_data.wheat; i++) available_resources.push_back("wheat");
    for (int i = 0; i < v_data.ore; i++) available_resources.push_back("ore");

    if (!available_resources.empty())
    {
        int res_index = UtilityFunctions::randi_range(0, available_resources.size() - 1);
        String stolen_res = available_resources[res_index];

        if (stolen_res == "wood") { v_data.wood--; s_data.wood++; }
        else if (stolen_res == "brick") { v_data.brick--; s_data.brick++; }
        else if (stolen_res == "sheep") { v_data.sheep--; s_data.sheep++; }
        else if (stolen_res == "wheat") { v_data.wheat--; s_data.wheat++; }
        else if (stolen_res == "ore") { v_data.ore--; s_data.ore++; }

        rpc("client_sync_resources", victim_id, v_data.wood, v_data.brick, v_data.sheep, v_data.wheat, v_data.ore);
        rpc("client_sync_resources", sender_id, s_data.wood, s_data.brick, s_data.sheep, s_data.wheat, s_data.ore);
    }
}

// ==========================================
// 8. 称号・勝利判定
// ==========================================

void CatanGame::update_longest_road()
{
    int best_player = 0;
    int best_length = 4;
    bool tie = false;

    for (int pid : player_order)
    {
        std::vector<String> p_edges;
        for (auto& ep : board_edges)
        {
            if (ep.second.owner_id == pid)
            {
                p_edges.push_back(ep.first);
            }
        }
        
        if (p_edges.empty())
        {
            continue;
        }

        int max_len_for_p = 0;

        auto dfs = [&](auto& self, const String& current_vertex, std::vector<String>& visited_edges) -> int
        {
            int max_l = 0;
            for (const String& edge : p_edges)
            {
                if (std::find(visited_edges.begin(), visited_edges.end(), edge) != visited_edges.end())
                {
                    continue;
                }

                if (board_edges[edge].midpoint.distance_to(board_vertices[current_vertex].position) < 35.0f)
                {
                    String next_vertex = "";
                    for (const auto& vp : board_vertices)
                    {
                        if (vp.first != current_vertex && board_edges[edge].midpoint.distance_to(vp.second.position) < 35.0f)
                        {
                            next_vertex = vp.first;
                            break;
                        }
                    }
                    if (next_vertex == "")
                    {
                        continue;
                    }

                    visited_edges.push_back(edge);

                    if (board_vertices[next_vertex].owner_id != 0 && board_vertices[next_vertex].owner_id != pid)
                    {
                        max_l = std::max(max_l, 1);
                    }
                    else
                    {
                        max_l = std::max(max_l, 1 + self(self, next_vertex, visited_edges));
                    }

                    visited_edges.pop_back();
                }
            }
            return max_l;
        };

        for (const String& start_edge : p_edges)
        {
            std::vector<String> end_vertices;
            for (const auto& vp : board_vertices)
            {
                if (board_edges[start_edge].midpoint.distance_to(vp.second.position) < 35.0f)
                {
                    end_vertices.push_back(vp.first);
                }
            }
            
            for (const String& start_v : end_vertices)
            {
                std::vector<String> visited;
                visited.push_back(start_edge);

                String next_v = (end_vertices[0] == start_v && end_vertices.size() > 1) ? end_vertices[1] : end_vertices[0];
                
                int len = 1;
                if (board_vertices[next_v].owner_id != 0 && board_vertices[next_v].owner_id != pid)
                {
                    // 相手の拠点で分断
                }
                else
                {
                    len += dfs(dfs, next_v, visited);
                }
                
                max_len_for_p = std::max(max_len_for_p, len);
            }
        }

        if (max_len_for_p > best_length)
        {
            best_length = max_len_for_p;
            best_player = pid;
            tie = false;
        }
        else if (max_len_for_p == best_length && max_len_for_p >= 5)
        {
            if (pid == longest_road_player)
            {
                best_player = pid;
                tie = false;
            }
            else if (best_player != longest_road_player)
            {
                tie = true;
            }
        }
    }

    if (tie)
    {
        best_player = 0;
    }

    if (best_player != longest_road_player)
    {
        longest_road_player = best_player;
        longest_road_length = best_player == 0 ? 4 : best_length;
        UtilityFunctions::print("Server: Longest Road holder changed to Player ", longest_road_player, " (length: ", longest_road_length, ")");
        rpc("client_notify_longest_road", longest_road_player);
    }
    else if (best_player != 0)
    {
        longest_road_length = best_length;
    }
}

void CatanGame::server_check_victory(int player_id)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }

    int total_vp = 0;

    for (const auto& pair : board_vertices)
    {
        if (pair.second.owner_id == player_id)
        {
            if (pair.second.building_type == 1) total_vp += 1;
            else if (pair.second.building_type == 2) total_vp += 2;
        }
    }

    total_vp += players[player_id].dev_vp;
    if (player_id == largest_army_player) total_vp += 2;
    if (player_id == longest_road_player) total_vp += 2;

    UtilityFunctions::print("Server: Player ", player_id, " currently has ", total_vp, " VPs.");

    if (total_vp >= 10)
    {
        UtilityFunctions::print("Server: Player ", player_id, " WINS THE GAME!");
        rpc("client_announce_winner", player_id);
    }
}

void CatanGame::client_announce_winner(int winner_id)
{
    emit_signal("game_won", winner_id);
}

void CatanGame::client_notify_largest_army(int player_id)
{
    emit_signal("largest_army_changed", player_id);
}

void CatanGame::client_notify_longest_road(int player_id)
{
    emit_signal("longest_road_changed", player_id);
}

// ==========================================
// 9. 切断・再接続
// ==========================================

void CatanGame::_on_peer_disconnected(int id)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    if (players.count(id) > 0)
    {
        players[id].is_connected = false;
        String d_name = players[id].player_name;
        rpc("client_notify_disconnect", d_name);

        if (players[id].is_waiting_for_discard)
        {
            PlayerData& p = players[id];
            int total_hand = p.wood + p.brick + p.sheep + p.wheat + p.ore;
            int amount_to_discard = total_hand / 2;

            for (int i = 0; i < amount_to_discard; i++)
            {
                if (p.wood > 0) p.wood--;
                else if (p.brick > 0) p.brick--;
                else if (p.sheep > 0) p.sheep--;
                else if (p.wheat > 0) p.wheat--;
                else if (p.ore > 0) p.ore--;
            }
            
            p.is_waiting_for_discard = false;
            pending_discard_players--;
            UtilityFunctions::print("Server: 切断されたプレイヤーの手札を強制没収しました。残り待機: ", pending_discard_players);

            if (pending_discard_players <= 0)
            {
                pending_discard_players = 0;
                int roller_id = player_order[current_turn_index];
                rpc_id(roller_id, "client_notify_robber_phase");
            }
        }

        if (player_order.size() > 0 && player_order[current_turn_index] == id)
        {
            int start_index = current_turn_index;
            do
            {
                current_turn_index = (current_turn_index + 1) % player_order.size();
            } while (!players[player_order[current_turn_index]].is_connected && current_turn_index != start_index);

            has_rolled_dice_this_turn = false;
            rpc("client_sync_turn", player_order[current_turn_index], current_phase);
        }
    }
}

void CatanGame::_on_peer_connected(int id)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    // 再接続時の復旧処理はフェーズ2へ
}

void CatanGame::client_notify_disconnect(const String& player_name)
{
    emit_signal("player_disconnected", player_name);
}

void CatanGame::request_reconnect(const String& old_name)
{
    rpc_id(1, "server_process_reconnect", old_name);
}

void CatanGame::server_process_reconnect(const String& old_name)
{
    if (!get_tree()->get_multiplayer()->is_server())
    {
        return;
    }
    
    int new_id = get_tree()->get_multiplayer()->get_remote_sender_id();
    if (new_id == 0)
    {
        new_id = 1;
    }

    int old_id = 0;
    for (auto& pair : players)
    {
        if (pair.second.player_name == old_name && !pair.second.is_connected)
        {
            old_id = pair.first;
            break;
        }
    }

    if (old_id == 0)
    {
        UtilityFunctions::print("Server: 復帰対象が見つかりません。名前: ", old_name);
        return;
    }

    UtilityFunctions::print("Server: Player Reconnecting... Old ID: ", old_id, " -> New ID: ", new_id);

    PlayerData p_data = players[old_id];
    p_data.is_connected = true;
    players[new_id] = p_data;
    players.erase(old_id);

    for (int i = 0; i < player_order.size(); i++)
    {
        if (player_order[i] == old_id)
        {
            player_order[i] = new_id;
            break;
        }
    }

    for (auto& pair : board_vertices)
    {
        if (pair.second.owner_id == old_id)
        {
            pair.second.owner_id = new_id;
        }
    }
    
    for (auto& pair : board_edges)
    {
        if (pair.second.owner_id == old_id)
        {
            pair.second.owner_id = new_id;
        }
    }

    if (largest_army_player == old_id) largest_army_player = new_id;
    if (longest_road_player == old_id) longest_road_player = new_id;
    if (trade_proposer_id == old_id) trade_proposer_id = new_id;

    Dictionary state;
    state["current_phase"] = current_phase;
    state["robber_pos"] = robber_pos;
    
    Array v_arr;
    for (auto& pair : board_vertices)
    {
        if (pair.second.owner_id != 0)
        {
            Dictionary v;
            v["name"] = pair.first;
            v["owner"] = pair.second.owner_id;
            v["type"] = pair.second.building_type;
            v_arr.push_back(v);
        }
    }
    state["vertices"] = v_arr;

    Array e_arr;
    for (auto& pair : board_edges)
    {
        if (pair.second.owner_id != 0)
        {
            Dictionary e;
            e["name"] = pair.first;
            e["owner"] = pair.second.owner_id;
            e_arr.push_back(e);
        }
    }
    state["edges"] = e_arr;

    rpc_id(new_id, "client_receive_full_state", state);
    rpc("client_sync_reconnect", old_id, new_id, old_name);

    Array player_info_list;
    for (int i = 0; i < player_order.size(); i++)
    {
        int pid = player_order[i];
        Dictionary info;
        info["id"] = pid;
        info["turn_index"] = players[pid].turn_index;
        info["name"] = players[pid].player_name;
        info["vp"] = 0;
        info["hand_count"] = players[pid].wood + players[pid].brick + players[pid].sheep + players[pid].wheat + players[pid].ore;
        info["dev_cards"] = players[pid].dev_cards;
        player_info_list.push_back(info);
    }
    
    rpc("client_sync_player_list", player_info_list);
    rpc("client_sync_resources", new_id, p_data.wood, p_data.brick, p_data.sheep, p_data.wheat, p_data.ore);
    rpc_id(new_id, "client_sync_private_dev_cards", p_data.dev_knight, p_data.dev_vp, p_data.dev_road, p_data.dev_plenty, p_data.dev_mono);
    rpc("client_sync_turn", player_order[current_turn_index], current_phase);
}

void CatanGame::client_sync_reconnect(int old_id, int new_id, const String& p_name)
{
    emit_signal("player_reconnected", old_id, new_id, p_name);
}

void CatanGame::client_receive_full_state(Dictionary state)
{
    emit_signal("full_state_received", state);
}