#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/button.hpp>

namespace godot
{
    class BattleScene : public Node3D
    {
        GDCLASS(BattleScene, Node3D)

    private:
        Marker3D* player_spawn_pos;
        Marker3D* enemy_spawn_pos;

        Button* rock_button;     // グー
        Button* scissors_button; // チョキ
        Button* paper_button;    // パー

        /**
         * @brief モンスター名に基づいて対応するモデルのパスを取得する
         * 
         * @param name モンスターの名前
         * @return モデルのパス
         */
        String _get_model_path_by_name(const String& name);

        int player_hp; // プレイヤーの体力
        int enemy_hp; // 敵の体力

        int loaded_player_count; // 読み込んだプレイヤー数

        int command_received_count; // 受信した人数
        bool has_selected;          // 自分が選択済みか

        String server_host_hand;   
        String server_client_hand;

    protected:
        static void _bind_methods();

    public:
        BattleScene(); // コンストラクタ
        ~BattleScene(); // デストラクタ

        virtual void _ready() override; // シーン準備完了時の処理
        
        void _rpc_notify_loaded(); // 読み込み完了報告用RPC
        void _rpc_start_spawning(); // スポーン開始合図用RPC
        /**
         * @brief 指定されたモンスター名に基づいて敵をスポーンさせる
         * * @param monster_name スポーンさせるモンスターの名前
         * @param is_player プレイヤー側か敵側か
         */
        void _rpc_spawn_enemy(const String& monster_name, bool is_player); // 引数を追加

        // ボタン処理
        void _on_rock_pressed(); // グー
        void _on_scissors_pressed(); // チョキ
        void _on_paper_pressed(); // パー
        /**
         * @brief プレイヤーの手をサーバーに送信する
         * 
         * @param hand プレイヤーが選択した手（"rock", "scissors", "paper"）
         */
        void _submit_hand(const String& hand);

        /**
         * @brief [RPC] プレイヤーの手を受信する
         * 
         * @param hand 受信した手（"rock", "scissors", "paper"）
         */
        void _rpc_submit_hand(const String& hand);

        // [RPC] 結果発表 & ダメージ適用
        // winner_side: 0=あいこ, 1=ホスト勝利, 2=クライアント勝利
        // first_attacker: あいこの場合、どっちが先に動くか (1=ホスト, 2=クライアント)
        void _rpc_resolve_janken(const String& h_hand, const String& c_hand, int winner_side, int first_attacker);

        // [RPC] 決着通知
        void _rpc_notify_defeat();
    };
}