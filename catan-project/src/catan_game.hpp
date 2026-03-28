#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/variant/vector2.hpp>
#include <godot_cpp/variant/packed_int32_array.hpp>
#include <map>
#include <vector>

namespace godot
{
    // --- データ構造 ---

    // 交差点（頂点）のデータ
    struct VertexData 
    {
        Vector2 position;      // 頂点の位置
        int owner_id = 0;      // 0: 空き地, 1以上: 持ち主のプレイヤーID
        int building_type = 0; // 0: なし, 1: 開拓地(家), 2: 都市
        String port_type = ""; // 港の種類（例: "3:1", "wood", "brick", ...）
    };

    // 辺（道）のデータ
    struct EdgeData 
    {
        Vector2 midpoint;      // この辺の中心座標
        int owner_id = 0;      // 0: 空き地, 1以上: 持ち主のプレイヤーID
    };

    // プレイヤーのデータ
    struct PlayerData 
    {
        // 資源
        int wood = 0;
        int brick = 0;
        int sheep = 0;
        int wheat = 0;
        int ore = 0;
        
        // ステータス
        int turn_index = 0;
        int dev_cards = 0;     // 全員に見える「合計枚数」
        String player_name = "Unknown";
        bool is_connected = true;
        bool is_waiting_for_discard = false;

        // 自分だけが見る種類ごとの発展カード枚数
        int dev_knight = 0;
        int dev_vp = 0;
        int dev_road = 0;
        int dev_plenty = 0;
        int dev_mono = 0;

        // カード・能力の使用状況
        int free_roads_available = 0;
        int knights_played = 0;
        bool has_played_dev_card_this_turn = false;
        int new_dev_knight = 0;
        int new_dev_road = 0;
        int new_dev_plenty = 0;
        int new_dev_mono = 0;
    };

    // ゲームの進行フェーズ
    enum GamePhase 
    {
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
        // --- プライベート変数 ---
        
        // 通信接続用のネットワークピア
        Ref<ENetMultiplayerPeer> peer;
        
        // 進行管理
        GamePhase current_phase = PHASE_SETUP_1;
        Vector2 robber_pos = Vector2(-9999, -9999);
        int setup_settlements_built_this_turn = 0;
        int setup_roads_built_this_turn = 0;
        int pending_discard_players = 0;

        // トレード管理
        bool is_player_trade_active = false;
        int trade_proposer_id = 0;
        int t_gw=0, t_gb=0, t_gs=0, t_gwh=0, t_go=0; 
        int t_ww=0, t_wb=0, t_ws=0, t_wwh=0, t_wo=0;

        // 称号管理
        int largest_army_player = 0;
        int largest_army_count = 2; // 3枚以上で獲得
        int longest_road_player = 0;
        int longest_road_length = 4; // 5本以上で獲得

    protected:
        // Godotへのメソッド登録用
        static void _bind_methods();

    public:
        // コンストラクタ
        CatanGame();
        // デストラクタ
        ~CatanGame();

        // --- パブリック変数 ---
        std::map<String, VertexData> board_vertices;
        std::map<String, EdgeData> board_edges;
        std::map<int, PlayerData> players;
        std::vector<int> player_order;
        std::vector<String> dev_card_deck;

        int current_turn_index = 0;
        bool has_rolled_dice_this_turn = false;

        // ==========================================
        // 1. ネットワーク・ライフサイクル管理
        // ==========================================

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

        // ゲーム開始
        void start_game();

        /**
         * @brief [RPC] 実際に全員の画面を切り替える
         * @param scene_path 切り替え先のシーンパス
         */
        void rpc_change_scene(const String& scene_path);

        /**
         * @brief プレイヤー名の登録
         * @param name プレイヤー名
         */
        void register_player_name(const String& name);

        // ==========================================
        // 2. ターン・進行管理
        // ==========================================

        // サーバーがゲーム開始時に呼ぶ
        void start_turn_system();
        
        // クライアントが「ターン終了」ボタンを押した時
        void request_end_turn();
        
        // サーバーがターン交代を処理する
        void server_process_end_turn();
        
        /**
         * @brief [RPC] 全員に「次はこの人のターンだよ」と知らせる
         * @param player_id 次のターンのプレイヤーID
         * @param phase 現在のフェーズ
         */
        void client_sync_turn(int player_id, int phase);
        
        /**
         * @brief [RPC] プレイヤーリストの同期
         * @param player_info_list プレイヤー情報の配列
         */
        void client_sync_player_list(Array player_info_list);

        // セットアップターンの進行
        void advance_setup_turn();

        // ==========================================
        // 3. ダイス・資源産出
        // ==========================================

        // ダイスロールの要求
        void request_roll_dice();

        /**
         * @brief [RPC] サーバーから全クライアントへダイスの結果を通知
         * @param dice1 1つ目のダイスの値
         * @param dice2 2つ目のダイスの値
         */
        void notify_dice_result(int dice1, int dice2);

        /**
         * @brief サーバー側で資源を増やし、全員に同期する
         * @param player_id 対象プレイヤーID
         * @param resource_type 資源の種類
         * @param amount 増加量
         */
        void add_resource(int player_id, const String& resource_type, int amount);
        
        /**
         * @brief [RPC] サーバーから全員へ「このプレイヤーの資源がこうなったよ」と教える
         */
        void client_sync_resources(int player_id, int wood, int brick, int sheep, int wheat, int ore);

        /**
         * @brief 六角形タイルから資源を分配する
         */
        void distribute_resources_for_hex(Vector2 hex_center, float hex_radius, const String& resource_type);

        // ==========================================
        // 4. 建築・ボード管理
        // ==========================================

        /**
         * @brief 交差点の座標を登録する
         * @param vertex_name 頂点名
         * @param pos 座標
         */
        void register_vertex(const String& vertex_name, Vector2 pos);

        /**
         * @brief 辺の座標を登録する
         * @param edge_name 辺名
         * @param midpoint 中心座標
         */
        void register_edge(const String& edge_name, Vector2 midpoint);

        /**
         * @brief 港を登録する
         * @param vertex_name 頂点名
         * @param port_type 港の種類
         */
        void register_port(const String& vertex_name, const String& port_type);

        // --- 家（開拓地） ---
        /** @brief [RPC] 建築要求 */
        void request_build_settlement(const String& vertex_name);
        /** @brief [RPC] サーバー処理 */
        void server_process_build(const String& vertex_name);
        /** @brief [RPC] クライアント同期 */
        void client_sync_build(const String& vertex_name, int player_id);

        // --- 道（街道） ---
        /** @brief [RPC] 建築要求 */
        void request_build_road(const String& edge_name);
        /** @brief [RPC] サーバー処理 */
        void server_process_build_road(const String& edge_name);
        /** @brief [RPC] クライアント同期 */
        void client_sync_build_road(const String& edge_name, int player_id);

        // --- 都市 ---
        /** @brief [RPC] 建築要求 */
        void request_build_city(const String& vertex_name);
        /** @brief [RPC] サーバー処理 */
        void server_process_build_city(const String& vertex_name);
        /** @brief [RPC] クライアント同期 */
        void client_sync_build_city(const String& vertex_name, int player_id);

        // ==========================================
        // 5. トレード（銀行・プレイヤー）
        // ==========================================

        /**
         * @brief クライアントが「これを払って、これをもらう」とお願いする
         * @param give_res 払う資源
         * @param get_res もらう資源
         */
        void request_bank_trade(const String& give_res, const String& get_res);
        void server_process_bank_trade(const String& give_res, const String& get_res);

        // プレイヤートレード
        void request_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);
        void server_process_propose_trade(int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);
        void client_receive_trade_proposal(int proposer_id, int gw, int gb, int gs, int gwh, int go, int ww, int wb, int ws, int wwh, int wo);

        void request_accept_trade();
        void server_process_accept_trade();
        void client_notify_trade_accepted(int accepter_id); 
        
        void request_execute_trade(int target_id);
        void server_process_execute_trade(int target_id);
        
        void request_cancel_trade();
        void server_process_cancel_trade();
        void client_trade_completed();

        // ==========================================
        // 6. 発展カード
        // ==========================================

        // 山札の準備
        void initialize_dev_deck();

        // カード購入
        void request_buy_dev_card();
        void server_process_buy_dev_card();
        
        /** @brief [RPC] 全員に「合計枚数が増えたよ」と知らせる */
        void client_sync_dev_card_bought(int player_id);
        
        /** @brief [RPC] 本人にだけ「各カードの所持枚数はこれだよ！」と教える */
        void client_sync_private_dev_cards(int knight, int vp, int road, int plenty, int mono);

        // カード使用（騎士）
        void request_play_knight();
        void server_process_play_knight();
        void client_prompt_knight_robber();

        // カード使用（独占）
        void request_play_monopoly(const String& res_type);
        void server_process_play_monopoly(const String& res_type);
        
        // カード使用（収穫）
        void request_play_plenty(const String& res1, const String& res2);
        void server_process_play_plenty(const String& res1, const String& res2);
        
        // カード使用（街道建設）
        void request_play_road_building();
        void server_process_play_road_building();
        void client_prompt_road_building();

        // ==========================================
        // 7. 盗賊・バースト・強奪
        // ==========================================

        /** @brief 初期位置の設定 */
        void set_initial_robber_pos(Vector2 pos);

        // 7が出た時の処理
        void server_process_roll_seven(int roller_id);
        
        // バースト（半分捨てる）
        void client_prompt_discard(int amount, int w, int b, int s, int wh, int o);
        void request_discard(int w, int b, int s, int wh, int o);
        void server_process_discard(int w, int b, int s, int wh, int o);
        
        // 盗賊フェーズへの移行
        void client_notify_robber_phase();

        // 盗賊の移動
        void request_move_robber(Vector2 pos);
        void server_process_move_robber(Vector2 pos);
        void client_sync_robber(Vector2 pos, Array victims);
        
        // 強奪
        void request_steal(int victim_id);
        void server_process_steal(int victim_id);

        // ==========================================
        // 8. 称号・勝利判定
        // ==========================================

        void update_longest_road();
        
        /** @brief [RPC] 勝利判定 */
        void server_check_victory(int player_id);
        void client_announce_winner(int winner_id);
        
        void client_notify_largest_army(int player_id);
        void client_notify_longest_road(int player_id);

        // ==========================================
        // 9. 切断・再接続
        // ==========================================

        void _on_peer_disconnected(int id);
        void _on_peer_connected(int id);
        
        /** @brief [RPC] 誰かが切断したことを通知 */
        void client_notify_disconnect(const String& player_name);

        /** @brief 再接続要求 */
        void request_reconnect(const String& old_name);
        void server_process_reconnect(const String& old_name);
        
        /** @brief [RPC] 復帰の同期 */
        void client_sync_reconnect(int old_id, int new_id, const String& p_name);
        void client_receive_full_state(Dictionary state);
    };

} // namespace godot