#pragma once

#include "skill_data.hpp"
#include "monster_data.hpp"

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/tween.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/canvas_layer.hpp>
#include <godot_cpp/classes/texture_rect.hpp>
#include <godot_cpp/classes/texture2d.hpp>
#include <godot_cpp/classes/control.hpp>

namespace godot
{
    class BattleScene : public Node3D
    {
        GDCLASS(BattleScene, Node3D)

    private:
        Marker3D* player_spawn_pos; // プレイヤーの出現位置
        Marker3D* enemy_spawn_pos;  // 敵の出現位置

        Button* skill_button_1;     // スキルボタンへのポインタ
        Button* skill_button_2;     // スキルボタンへのポインタ
        Button* skill_button_3;     // スキルボタンへのポインタ

        TypedArray<SkillData> current_skills;   // 現在使用可能なスキルリスト

        /**
         * @brief 名前からモデルのパスを取得する
         * @param name モンスターの名前
         * @return String モデルのリソースパス
         */
        String _get_model_path_by_name(const String& name);

        /**
         * @brief 手のタイプ(int)を文字列に変換する
         * @param type 手のタイプ (0:rock, 1:scissors, 2:paper)
         * @return String 文字列としての手
         */
        String _hand_type_to_string(int type);

        /**
         * @brief スキルボタンのテキストと有効状態を更新する
         */
        void _update_ui_buttons();

        int player_hp;      // プレイヤーの現在HP
        int enemy_hp;       // 敵の現在HP

        int loaded_player_count;    // ロード完了を通知したプレイヤー数
        int command_received_count; // コマンド受信済みプレイヤー数
        bool has_selected;          // プレイヤーが手を選択済みかどうか

        String server_host_hand;    // サーバー側で保持するホストの手
        String server_client_hand;  // サーバー側で保持するクライアントの手

        /**
         * @brief 攻撃の演出（移動、アニメーション、ダメージ適用）を実行する
         * @param attacker 攻撃側のノード
         * @param target 防御側のノード
         * @param skill 使用するスキルデータ
         * @param p_hand プレイヤーの手 ("rock", "scissors", "paper")
         * @param e_hand 敵の手 ("rock", "scissors", "paper
         */
        void _perform_attack_sequence(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill, String p_hand, String e_hand);

        /**
         * @brief 出した手に対応するスキルをリストから探す
         * @param skills スキルリスト
         * @param hand_str 出した手 ("rock", "scissors", "paper")
         * @return Ref<SkillData> 見つかったスキルデータ（なければnullptr）
         */
        Ref<SkillData> _get_skill_by_hand(const TypedArray<SkillData>& skills, const String& hand_str);

        /**
         * @brief 指定したパスのモデルを指定した親ノードに生成する
         * @param path モデルのリソースパス
         * @param parent_node 生成先の親ノード
         * @param is_player プレイヤー側かどうか（向きの調整用）
         */
        void _spawn_model_at(const String& path, Node3D* parent_node, bool is_player);

        Dictionary p1_data;
        Dictionary p2_data;
        TypedArray<SkillData> enemy_player_skills;
        
        bool battle_ready;

        // --- UIノードへのポインタ ---
        ProgressBar* player_hp_bar;
        ProgressBar* enemy_hp_bar;
        Label* message_label;
        CanvasLayer* ui_layer;

        int player_max_hp;
        int enemy_max_hp;

        bool is_transitioning = false;

        Control* janken_effect_root;
        TextureRect* left_hand_rect;
        TextureRect* right_hand_rect;
        /** @brief じゃんけんUIを表示する
         * @param p_hand プレイヤーの手 ("rock", "scissors", "paper")
         * @param e_hand 敵の手 ("rock", "scissors", "paper")
         */
        void _show_janken_ui(String p_hand, String e_hand);
        // じゃんけんUIを非表示にする
        void _hide_janken_ui();
        
        // 画像リソースを保持（毎回ロードするのを避けるため）
        Ref<Texture2D> tex_rock;
        Ref<Texture2D> tex_scissors;
        Ref<Texture2D> tex_paper;

        // 演出後に実行する攻撃データを一時保存
        Node3D* temp_attacker;
        Node3D* temp_defender;
        Ref<SkillData> temp_skill;

        /**
         * @brief じゃんけんのUI演出を再生する
         * @param my_hand 自分の手
         * @param enemy_hand 敵の手
         * @param winner_side 勝者のサイド (0:引き分け, 1:ホスト勝利, 2:クライアント勝利)
         * @param on_finished 演出終了後に呼ばれるコールバック
         */
        void _play_janken_ui_effect(const String& my_hand, const String& enemy_hand, int winner_side, Callable on_finished);

        void _start_attack_after_ui(); // 演出終了後に呼ばれる関数

        /**
         * @brief ダメージ計算の結果をHPとUIに適用する
         * @param attacker 攻撃側のノード
         * @param target 防御側のノード
         * @param skill 使用したスキル
         */
        void _apply_damage(Node3D* attacker, Node3D* target, const Ref<SkillData>& skill);

        /**
         * @brief バトル終了条件をチェックし、終了していれば処理を行う
         */
        void BattleScene::_check_battle_end();

        /**
         * @brief バトル画面のメッセージウィンドウにテキストを表示する
         * @param text 表示する文字列
         */
        void show_message(const String& text);

        /**
         * @brief [RPC] ホストから通知された最終的なダメージをHPバーに適用する
            * @param target_side ダメージを受けた側 (1:プレイヤー, 2:敵)
            * @param final_damage 適用するダメージ量
         */
        void _rpc_sync_hp(int target_side, int final_damage);

    protected:
        static void _bind_methods();

    public:
        BattleScene();
        ~BattleScene();

        virtual void _ready() override;
        
        /**
         * @brief ロード完了をサーバーに通知し、自身のデータを登録する
         */
        void _rpc_notify_loaded();

        void _rpc_start_spawning();

        /**
         * @brief サーバー側でプレイヤーの基本データを保持する
         * @param peer_id プレイヤーのピアID
         * @param model_path モデルのリソースパス
         * @param hp モンスターのHP
         * @param speed モンスターの素早さ
         */
        void _rpc_register_player_data(int peer_id, const String& model_path, int hp, int speed);

        /**
         * @brief 全員の準備が整った際、サーバーから送られるセットアップ命令
         * @param host_info ホストプレイヤーの情報
         * @param client_info クライアントプレイヤーの情報
         */
        void _rpc_setup_battle(const Dictionary& host_info, const Dictionary& client_info);

        void _on_skill_1_pressed();
        void _on_skill_2_pressed();
        void _on_skill_3_pressed();

        /**
         * @brief 選択した手をサーバーに送信する
         */
        void _submit_hand(const String& hand);

        /**
         * @brief [RPC] サーバー側で各プレイヤーの手を受理する
         * @param hand プレイヤーが選択した手
         */
        void _rpc_submit_hand(const String& hand);

        /**
         * @brief [RPC] サーバーがじゃんけんの結果を判定し、全員に通知する
         * @param h_hand ホストの手
         * @param c_hand クライアントの手
         * @param winner_side 勝者のサイド (0:引き分け, 1:ホスト勝利, 2:クライアント勝利)
         * @param first_attacker 先攻のサイド (1:ホスト, 2:クライアント)
         */
        void _rpc_resolve_janken(const String& h_hand, const String& c_hand, int winner_side, int first_attacker);

        /**
         * @brief [RPC] 敗北（決着）を通知する
         */
        void _rpc_notify_defeat();

        /**
         * @brief サーバーに最新のバトルデータを要求する
         */
        void request_battle_data();
    };
}