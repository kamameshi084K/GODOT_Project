#include "title_screen.hpp"
#include "game_manager.hpp"

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void TitleScreen::_bind_methods()
{
    // メインメニューのボタン
    ClassDB::bind_method(D_METHOD("_on_start_button_pressed"), &TitleScreen::_on_start_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_multiplayer_button_pressed"), &TitleScreen::_on_multiplayer_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_quit_button_pressed"), &TitleScreen::_on_quit_button_pressed);

    // --- ネットワークメニュー ---
    ClassDB::bind_method(D_METHOD("_on_host_button_pressed"), &TitleScreen::_on_host_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_join_button_pressed"), &TitleScreen::_on_join_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_back_button_pressed"), &TitleScreen::_on_back_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_connected_to_server"), &TitleScreen::_on_connected_to_server);
    ClassDB::bind_method(D_METHOD("_on_connection_failed"), &TitleScreen::_on_connection_failed);
    
    // 3択ボタンの登録
    ClassDB::bind_method(D_METHOD("_on_fire_button_pressed"), &TitleScreen::_on_fire_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_water_button_pressed"), &TitleScreen::_on_water_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_grass_button_pressed"), &TitleScreen::_on_grass_button_pressed);

    // エディタ設定用パス
    ClassDB::bind_method(D_METHOD("set_main_menu_path", "path"), &TitleScreen::set_main_menu_path);
    ClassDB::bind_method(D_METHOD("get_main_menu_path"), &TitleScreen::get_main_menu_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "main_menu_path"), "set_main_menu_path", "get_main_menu_path");

    ClassDB::bind_method(D_METHOD("set_selection_menu_path", "path"), &TitleScreen::set_selection_menu_path);
    ClassDB::bind_method(D_METHOD("get_selection_menu_path"), &TitleScreen::get_selection_menu_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "selection_menu_path"), "set_selection_menu_path", "get_selection_menu_path");

    // ネットワークメニュー用パス
    ClassDB::bind_method(D_METHOD("set_network_menu_path", "path"), &TitleScreen::set_network_menu_path);
    ClassDB::bind_method(D_METHOD("get_network_menu_path"), &TitleScreen::get_network_menu_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "network_menu_path"), "set_network_menu_path", "get_network_menu_path");

    ClassDB::bind_method(D_METHOD("set_ip_input_path", "path"), &TitleScreen::set_ip_input_path);
    ClassDB::bind_method(D_METHOD("get_ip_input_path"), &TitleScreen::get_ip_input_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "ip_input_path"), "set_ip_input_path", "get_ip_input_path");
}

TitleScreen::TitleScreen()
{
    main_menu = nullptr;
    selection_menu = nullptr;
    network_menu = nullptr;
    ip_input = nullptr;
}

TitleScreen::~TitleScreen()
{
}

void TitleScreen::_ready()
{
    // パスを使って実際のノードを取得
    if (!main_menu_path.is_empty())
    {
        main_menu = get_node<Control>(main_menu_path);
    }
    
    if (!selection_menu_path.is_empty())
    {
        selection_menu = get_node<Control>(selection_menu_path);
    }
    if (!network_menu_path.is_empty())
    {
        network_menu = get_node<Control>(network_menu_path);
    }
    if (!ip_input_path.is_empty())
    {
        ip_input = get_node<LineEdit>(ip_input_path);
    }

    if (get_tree() && get_tree()->get_multiplayer().is_valid()) // is_valid()チェックを追加するとより安全です
    {
        Ref<MultiplayerAPI> mp = get_tree()->get_multiplayer();
        if (mp.is_valid())
        {
            // すでに接続されていないか確認してから接続
            if (!mp->is_connected("connected_to_server", Callable(this, "_on_connected_to_server")))
            {
                mp->connect("connected_to_server", Callable(this, "_on_connected_to_server"));
            }
            if (!mp->is_connected("connection_failed", Callable(this, "_on_connection_failed")))
            {
                mp->connect("connection_failed", Callable(this, "_on_connection_failed"));
            }
        }
    }

    // 初期状態：メインメニューだけ表示、選択画面は隠す
    if (main_menu) main_menu->show();
    if (selection_menu) selection_menu->hide();
    if (network_menu) network_menu->hide();

    // コントローラー用にフォーカスを当てる
    if (main_menu)
    {
        // ※ノード名が違う場合は調整してください
        Node* btn = main_menu->find_child("StartButton", true, false);
        if (btn)
        {
            Button* start_btn = Object::cast_to<Button>(btn);
            if (start_btn) start_btn->grab_focus();
        }
    }

    
}

void TitleScreen::_on_start_button_pressed()
{
    // いきなりゲーム開始せず、メニューを切り替える
    if (main_menu) main_menu->hide();
    if (selection_menu) 
    {
        selection_menu->show();
        
        // 選択肢の最初のボタンにフォーカスを当てる
        Node* btn = selection_menu->find_child("FireButton", true, false);
        if (btn)
        {
            Button* fire_btn = Object::cast_to<Button>(btn);
            if (fire_btn) fire_btn->grab_focus();
        }
    }
}

void TitleScreen::_on_multiplayer_button_pressed()
{
    // マルチプレイ：ネットワーク設定画面へ
    if (main_menu) main_menu->hide();
    if (network_menu) 
    {
        network_menu->show();
        // HostButtonにフォーカス
        Node* btn = network_menu->find_child("HostButton", true, false);
        if (btn) Object::cast_to<Button>(btn)->grab_focus();
    }
}

void TitleScreen::_on_quit_button_pressed()
{
    get_tree()->quit();
}

// --- ネットワークメニュー ---

void TitleScreen::_on_host_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        // サーバーを立てる
        gm->host_game();
        
        // そのままモンスター選択へ進む
        if (network_menu) network_menu->hide();
        if (selection_menu) selection_menu->show();
    }
}

void TitleScreen::_on_join_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm && ip_input)
    {
        String ip = ip_input->get_text();
        if (ip.is_empty()) ip = "127.0.0.1";
        
        // 接続を開始するだけ（画面切り替えはまだしない！）
        gm->join_game(ip);
        
        // ここで「Connecting...」みたいなラベルを表示すると親切です
        UtilityFunctions::print("Connecting to ", ip, "...");
        /*
        if (network_menu) network_menu->hide();
        if (selection_menu) selection_menu->show();
        */
    }
}

void TitleScreen::_on_connected_to_server()
{
    UtilityFunctions::print("Successfully Connected!");
    
    // つながったので、ここで初めて画面を切り替える
    if (network_menu) network_menu->hide();
    if (selection_menu) selection_menu->show();
}

void TitleScreen::_on_connection_failed()
{
    UtilityFunctions::print("Connection Failed!");
    // ここで「接続失敗」のメッセージを出したり、joinボタンを再度押せるように戻したりする
    // 今回はとりあえず、GameManager側で接続をリセットする処理が必要ですが、まずはログを出すだけにします
}

void TitleScreen::_on_back_button_pressed()
{
    if (network_menu) network_menu->hide();
    if (main_menu) 
    {
        main_menu->show();
        Node* btn = main_menu->find_child("StartButton", true, false);
        if (btn) Object::cast_to<Button>(btn)->grab_focus();
    }
}

void TitleScreen::_on_fire_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm) gm->select_starter_monster(0); // 0:炎

    // ゲーム開始処理へ
    _attempt_start_game();
}

void TitleScreen::_on_water_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm) gm->select_starter_monster(1); // 1:水

    // ゲーム開始処理へ
    _attempt_start_game();
}

void TitleScreen::_on_grass_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm) gm->select_starter_monster(2); // 2:草

    // ゲーム開始処理へ
    _attempt_start_game();
}

void TitleScreen::_attempt_start_game()
{
    GameManager* gm = GameManager::get_singleton();

    // ネットワーク接続状態を確認
    bool is_online = false;
    if (get_tree()->get_multiplayer().is_valid() && get_tree()->get_multiplayer()->has_multiplayer_peer())
    {
        is_online = true;
    }

    if (is_online)
    {
        // --- オンラインの場合 ---
        if (get_tree()->get_multiplayer()->is_server())
        {
            // ホストなら：ゲームサイクルを開始する（全員に通知が飛ぶ）
            if (gm) gm->start_collection_phase();
        }
        else
        {
            // クライアントなら：何もしない（ホストが開始するのを待つ）
            UtilityFunctions::print("Selected monster. Waiting for host to start...");
            // ここで「ホストの開始待ちです...」のようなラベルを表示すると親切です
        }
    }
    else
    {
        // --- ソロプレイ（オフライン）の場合 ---
        // 直接ワールド（収集フェーズ）へ移動
        get_tree()->change_scene_to_file("res://world.tscn");
    }
}

void TitleScreen::set_main_menu_path(const NodePath& path) { main_menu_path = path; }
NodePath TitleScreen::get_main_menu_path() const { return main_menu_path; }

void TitleScreen::set_selection_menu_path(const NodePath& path) { selection_menu_path = path; }
NodePath TitleScreen::get_selection_menu_path() const { return selection_menu_path; }

void TitleScreen::set_network_menu_path(const NodePath& path) { network_menu_path = path; }
NodePath TitleScreen::get_network_menu_path() const { return network_menu_path; }

void TitleScreen::set_ip_input_path(const NodePath& path) { ip_input_path = path; }
NodePath TitleScreen::get_ip_input_path() const { return ip_input_path; }
