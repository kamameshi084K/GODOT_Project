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
        
        String player_name = "Unknown";
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

        // 初期配置フェーズ中、そのターンに建てた数を記録する変数
        int setup_settlements_built_this_turn = 0;
        int setup_roads_built_this_turn = 0;

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
         * @param roll_value 決定されたダイスの合計値
         */
        void notify_dice_result(int roll_value);

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
    };

} // namespace godot