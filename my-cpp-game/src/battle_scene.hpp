#pragma once

#include "skill_data.hpp" // SkillData を使うので必須
#include "monster_data.hpp" // MonsterData も必要

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/tween.hpp>

namespace godot
{
    class BattleScene : public Node3D
    {
        GDCLASS(BattleScene, Node3D)

    private:
        Marker3D* player_spawn_pos;
        Marker3D* enemy_spawn_pos;

        Button* skill_button_1;
        Button* skill_button_2;
        Button* skill_button_3;

        TypedArray<SkillData> current_skills;

        /**
         * @brief モンスター名に基づいて対応するモデルのパスを取得する
         * 
         * @param name モンスターの名前
         * @return モデルのパス
         */
        String _get_model_path_by_name(const String& name);

        /**
         * @brief じゃんけんの手のタイプを文字列に変換する
         * 
         * @param type HandType のいずれか
         * @return 変換された文字列 ("rock", "scissors", "paper")
         */
        String _hand_type_to_string(int type);
        void _update_ui_buttons(); // ボタンのテキスト更新用

        int player_hp; // プレイヤーの体力
        int enemy_hp; // 敵の体力

        int loaded_player_count; // 読み込んだプレイヤー数

        int command_received_count; // 受信した人数
        bool has_selected;          // 自分が選択済みか

        String server_host_hand;   
        String server_client_hand;

        /**
         * @brief 攻撃アニメーションとダメージ処理を実行する内部関数
         * 
         * @param attacker 攻撃者のNode3D
         * @param target 攻撃対象のNode3D
         * @param skill 使用する技のデータ
         */
        void _perform_attack_sequence(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill);

        /**
         * @brief 手の文字列に基づいて対応するSkillDataを取得する
         * 
         * @param skills プレイヤーのスキルリスト
         * @param hand_str "rock", "scissors", "paper" のいずれか
         * @return 対応するSkillDataの参照
         */
        Ref<SkillData> _get_skill_by_hand(const TypedArray<SkillData>& skills, const String& hand_str);

        /**
         * @brief 指定されたパスのモデルを指定ノードにスポーンさせる
         * 
         * @param path モデルのリソースパス
         * @param parent_node スポーン先の親ノード
         * @param is_player プレイヤー側か敵側か
         */
        void _spawn_model_at(const String& path, Node3D* parent_node, bool is_player);

        Dictionary p1_data; // ホストの情報
        Dictionary p2_data; // クライアントの情報
        Dictionary enemy_player_info; 
        TypedArray<SkillData> enemy_player_skills; // 相手の技判定用
        
        bool battle_ready; // 戦闘開始準備完了フラグ

    protected:
        static void _bind_methods();

    public:
        BattleScene(); // コンストラクタ
        ~BattleScene(); // デストラクタ

        virtual void _ready() override; // シーン準備完了時の処理
        
        void _rpc_notify_loaded(); // 読み込み完了報告用RPC
        void _rpc_start_spawning(); // スポーン開始合図用RPC

        // 自分のモンスター情報をホストに送るRPC
        void _rpc_register_player_data(int peer_id, const String& model_path, int hp, int speed);

        // ホストから全員へ、バトルの配役を伝えるRPC
        // host_path: ホストのモデル, client_path: クライアントのモデル
        void _rpc_setup_battle(const Dictionary& host_info, const Dictionary& client_info);

        // ボタン処理
        void _on_skill_1_pressed();
        void _on_skill_2_pressed();
        void _on_skill_3_pressed();
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

        void request_battle_data();
    };
}