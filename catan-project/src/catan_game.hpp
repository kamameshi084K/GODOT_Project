#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <map>
#include <vector> // ★ 追加：プレイヤーリストの管理用
#include <godot_cpp/variant/packed_int32_array.hpp>


namespace godot
{
    struct VertexData {
        Vector2 position;      // 頂点の位置（例: "A", "B", ... に対応）
        int owner_id = 0;      // 0: 空き地, 1以上: 持ち主のプレイヤーID
        int building_type = 0; // 0: なし, 1: 開拓地(家), 2: 都市
        String port_type = ""; // 港の種類（例: "3:1", "wood", "brick", ...）
    };

    struct EdgeData {
        Vector2 midpoint;      // ★ 追加：この辺の中心座標
        int owner_id = 0;      
    };

    struct PlayerData {
        int wood = 0;
        int brick = 0;
        int sheep = 0;
        int wheat = 0;
        int ore = 0;
        
        int turn_index = 0;
        int dev_cards = 0; // 全員に見える「合計枚数」用

    // ★追加：自分だけが見る種類ごとの枚数
        int dev_knight = 0;
        int dev_vp = 0;
        int dev_road = 0;
        int dev_plenty = 0;
        int dev_mono = 0;
        int free_roads_available = 0;
        int knights_played = 0;
        bool has_played_dev_card_this_turn = false; // 今ターンすでに使ったか
        int new_dev_knight = 0;  // 今ターン買ったばかりの騎士
        int new_dev_road = 0;    // 今ターン買ったばかりの街道建設
        int new_dev_plenty = 0;  // 今ターン買ったばかりの収穫
        int new_dev_mono = 0;    // 今ターン買ったばかりの独占
        String player_name = "Unknown";
        bool is_connected = true;
        bool is_waiting_for_discard = false;
    };

    // ゲームの進行フェーズを表す列挙型（enum）を定義
        enum GamePhase {
            PHASE_SETUP_1, // 初期配置1巡目 (1→2→3→4)
            PHASE_SETUP_2, // 初期配置2巡目 (4→3→2→1)
            PHASE_MAIN     // 通常ゲーム (サイコロを振って進行)
        };

    /**
     * @class CatanGame
     * @brief マルチプレイヤー通信とゲーム進行（ダイスロール等）を管理するクラス
     */
    class CatanGame : public Node
    {
        GDCLASS(CatanGame, Node)

    private:
        // 通信接続用のネットワークピア
        Ref<ENetMultiplayerPeer> peer;
        GamePhase current_phase = PHASE_SETUP_1; // 現在のフェーズ

        Vector2 robber_pos = Vector2(-9999, -9999);

        // 初期配置フェーズ中、そのターンに建てた数を記録する変数
        int setup_settlements_built_this_turn = 0;
        int setup_roads_built_this_turn = 0;
        int pending_discard_players = 0;

        bool is_player_trade_active = false;
        int trade_proposer_id = 0;
        int t_gw=0, t_gb=0, t_gs=0, t_gwh=0, t_go=0; 
        int t_ww=0, t_wb=0, t_ws=0, t_wwh=0, t_wo=0;
        int largest_army_player = 0;
        int largest_army_count = 2; // 3枚以上で獲得
        int longest_road_player = 0;
        int longest_road_length = 4; // 5本以上で獲得

    protected:
        // Godotへのメソッド・RPC設定の登録用
        static void _bind_methods();

    public:
        // コンストラクタ
        CatanGame();
        // デストラクタ
        ~CatanGame();

        /**
         * @brief サーバーとしてゲームホストを開始
         * @param port 待ち受けポート番号
         */
        void host_game(int port = 53000);

        /**
         * @brief クライアントとして既存のサーバーに接続
         * @param address 接続先IPアドレス
         * @param port 接続先ポート番号
         */
        void join_game(const String& address, int port = 53000);

        /**
         * @brief [RPC] クライアントからサーバーへダイスロールを要求
         * @note サーバー権限でのみロジックが実行されます
         */
        void request_roll_dice();

        /**
         * @brief [RPC] サーバーから全クライアントへダイスの結果を通知
         * @param dice1 1つ目のダイスの値
         * @param dice2 2つ目のダイスの値
         */
        void notify_dice_result(int dice1, int dice2);

        void start_game();

        // [RPC] 実際に全員の画面を切り替える
        void rpc_change_scene(const String& scene_path);

        void request_build_settlement(const String& vertex_name);

        // 2. [RPC] サーバーだけが受け取る処理関数
        void server_process_build(const String& vertex_name);

        // 3. [RPC] サーバーから全員へ「ここに家を建てろ！」と命令する関数
        void client_sync_build(const String& vertex_name, int player_id);

        // 街道（辺）建築用の関数
        void request_build_road(const String& edge_name);
        void server_process_build_road(const String& edge_name);
        void client_sync_build_road(const String& edge_name, int player_id);

        std::map<String, VertexData> board_vertices;
        std::map<String, EdgeData> board_edges;

        void register_vertex(const String& vertex_name, Vector2 pos);

        std::map<int, PlayerData> players;

        // サーバー側で資源を増やし、全員に同期する関数
        void add_resource(int player_id, const String& resource_type, int amount);
        
        // [RPC] サーバーから全員へ「このプレイヤーの資源がこうなったよ」と教える関数
        void client_sync_resources(int player_id, int wood, int brick, int sheep, int wheat, int ore);

        void distribute_resources_for_hex(Vector2 hex_center, float hex_radius, const String& resource_type);

        void register_edge(const String& edge_name, Vector2 midpoint);

        std::vector<int> player_order; // プレイヤーIDの順番リスト
        int current_turn_index = 0;    // 今リストの何番目の人のターンか

        // ★ 追加：ターン管理用の関数
        void start_turn_system();      // サーバーがゲーム開始時に呼ぶ
        void request_end_turn();       // クライアントが「ターン終了」ボタンを押した時
        void server_process_end_turn();// サーバーがターン交代を処理する
        void client_sync_turn(int player_id, int phase); // 全員に「次はこの人のターンだよ」と知らせる
        bool has_rolled_dice_this_turn = false; // ターン中にダイスを振ったかどうかのフラグ
        void request_move_robber(Vector2 pos);
        void server_process_move_robber(Vector2 pos);
        void client_sync_robber(Vector2 pos, Array victims);
        void client_sync_player_list(Array player_info_list);
        void request_steal(int victim_id);
        void server_process_steal(int victim_id);
        void register_player_name(const String& name);
        void advance_setup_turn();
        void request_build_city(const String& vertex_name);
        void server_process_build_city(const String& vertex_name);
        void client_sync_build_city(const String& vertex_name, int player_id);
        // クライアントが「これを4つ払って、これを1つもらう！」とお願いする関数
        void request_bank_trade(const String& give_res, const String& get_res);

        // サーバーがそのお願いを受け取って、実際に計算する関数
        void server_process_bank_trade(const String& give_res, const String& get_res);
        void request_buy_dev_card();
        void server_process_buy_dev_card();

        // 全員に「この人がカードを買って合計枚数が増えたよ」と知らせる用
        void client_sync_dev_card_bought(int player_id);

        // ★追加：引いた本人にだけ「今の君の各カードの所持枚数はこれだよ！」と教える用
        void client_sync_private_dev_cards(int knight, int vp, int road, int plenty, int mono);
        std::vector<String> dev_card_deck; // 山札
        void initialize_dev_deck();        // 山札を準備する関数
        void request_play_knight();
        void server_process_play_knight();
        void client_prompt_knight_robber();
        void server_process_roll_seven(int roller_id);
        void client_prompt_discard(int amount, int w, int b, int s, int wh, int o);
        void request_discard(int w, int b, int s, int wh, int o);
        void server_process_discard(int w, int b, int s, int wh, int o);
        void client_notify_robber_phase();
        void request_play_monopoly(const String& res_type);
        void server_process_play_monopoly(const String& res_type);
        
        void request_play_plenty(const String& res1, const String& res2);
        void server_process_play_plenty(const String& res1, const String& res2);
        void request_play_road_building();
        void server_process_play_road_building();
        void client_prompt_road_building();
        void server_check_victory(int player_id);
        void client_announce_winner(int winner_id);
        void register_port(const String& vertex_name, const String& port_type);
        // ▼ プレイヤートレード用の関数
        void request_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);
        void server_process_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);
        void client_receive_trade_proposal(int proposer_id, int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);

        void request_accept_trade();
        void server_process_accept_trade();
        
        // ▼ 承諾者の通知と最終決定用
        void client_notify_trade_accepted(int accepter_id); 
        void request_execute_trade(int target_id);
        void server_process_execute_trade(int target_id);
        
        void request_cancel_trade();
        void server_process_cancel_trade();
        void client_trade_completed();

        void update_longest_road();
        
        void client_notify_largest_army(int player_id);
        void client_notify_longest_road(int player_id);
        void set_initial_robber_pos(Vector2 pos);

        void _on_peer_disconnected(int id);
        void _on_peer_connected(int id);
        void client_notify_disconnect(const String& player_name);

        void request_reconnect(const String& old_name);
        void server_process_reconnect(const String& old_name);
        void client_sync_reconnect(int old_id, int new_id, const String& p_name);
        void client_receive_full_state(Dictionary state);
    };

} // namespace godot