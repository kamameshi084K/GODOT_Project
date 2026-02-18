#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>

namespace godot
{
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
    };

} // namespace godot