#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/progress_bar.hpp>
#include <godot_cpp/classes/button.hpp>
#include <godot_cpp/classes/timer.hpp>
#include <godot_cpp/classes/panel_container.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    class BattleUI : public Control
    {
        // Godotのマクロ
        GDCLASS(BattleUI, Control)

    private:
        // 戦闘に関するステータス変数
        // プレイヤー側ステータス
        int player_hp; // プレイヤーのHP
        int max_player_hp; // プレイヤーの最大HP
        int player_attack_power; // プレイヤー攻撃力
        int player_defense_power; // プレイヤー防御力

        // 敵側ステータス
        int enemy_hp; // 敵のHP
        int enemy_attack_power; // 敵の攻撃力
        int enemy_defense_power; // 敵の防御力
        String enemy_name;      // 敵の名前

        // --- UIノードへのポインタ ---
        NodePath enemy_anim_path; // エディタで設定する「敵のアニメーションプレイヤーへのパス」
        AnimationPlayer *enemy_anim; // 実際のポインタ

        NodePath player_anim_path; // エディタで設定する「プレイヤーのアニメーションプレイヤーへのパス」
        AnimationPlayer *player_anim; // 実際のポインタ

        ProgressBar *player_hp_bar; // HP表示ラベルへのポインタ

        Button *attack_button;
        Button *run_button;

        // シーケンサー用のアニメーションプレイヤーポインタ
        AnimationPlayer *sequencer;

        PanelContainer *command_window; // コマンド選択ウィンドウ
        PanelContainer *message_window; // メッセージ表示ウィンドウ
        Label *message_label;           // メッセージの文字を表示するラベル

    protected:
        /**
         * @brief Godotのメソッドバインド用関数
         * 
         */
        static void _bind_methods();

    public:
        BattleUI();
        ~BattleUI();

        /**
         * @brief 画面が表示された瞬間に呼ばれる
         */
        virtual void _ready() override;

        /**
         * @brief 「攻撃ボタン」が押されたときに呼ばれる
         * @note エディタのシグナル接続画面から接続される想定
         */
        void _on_attack_button_pressed();

        /**
         * @brief 「逃げるボタン」が押されたときに呼ばれる
         * @note エディタのシグナル接続画面から接続される想定
         */
        void _on_run_button_pressed();
        
        /**
         * @brief 敵のアニメーションが終わった時に呼ばれる（死亡演出待ち用）
         * @param anim_name 終わったアニメーションの名前
         * @note エディタのシグナル接続画面から接続される想定
         */
        void _on_enemy_animation_finished(const StringName &anim_name);
        
        /**
         * @brief プレイヤーのアニメーションが終わった時に呼ばれる（攻撃演出待ち用）
         * @param anim_name 終わったアニメーションの名前
         * @note エディタのシグナル接続画面から接続される想定
         */
        void _on_player_animation_finished(const StringName &anim_name);

        /**
         * @brief 敵のHP表示ラベルを更新
         */
        void update_hp_label();


        /**
         * @brief 敵のアニメーションプレイヤーへのパスを設定
         * @param path 
         * @return NodePath
         * @note エディタから設定できるようにするためのセッター・ゲッター
         */
        void set_enemy_anim_path(const NodePath &path);

        /**
         * @brief 敵のアニメーションプレイヤーへのパスを取得
         * @return NodePath
         * @note エディタから設定できるようにするためのセッター・ゲッター
         */
        NodePath get_enemy_anim_path() const;

        /**
         * @brief プレイヤーのアニメーションプレイヤーへのパスを設定
         * @param path 
         * @return NodePath
         * @note エディタから設定できるようにするためのセッター・ゲッター
         */
        void set_player_anim_path(const NodePath &path);

        /**
         * @brief プレイヤーのアニメーションプレイヤーへのパスを取得
         * @return NodePath
         * @note エディタから設定できるようにするためのセッター・ゲッター
         */
        NodePath get_player_anim_path() const;

        /**
         * @brief 攻撃開始の演出（メッセージ表示とプレイヤーのアニメ再生）
         * 
         */
        void seq_player_attack_start();
        
        /**
         * @brief ダメージ計算と反映（HP減少と敵のリアクション）
         * 
         */
        void seq_deal_damage();

        /**
         * @brief 敵の攻撃開始の演出（メッセージ表示と敵のアニメ再生）
         * 
         */
        void seq_enemy_attack_start();

        /**
         * @brief 敵のダメージ計算と反映（HP減少とプレイヤーのリアクション）
         * 
         */
        void seq_enemy_deal_damage();
        
        /**
         * @brief プレイヤーターン終了（敵ターンへ移行）
         * 
         */
        void seq_end_player_turn();

        /**
         * @brief 敵ターン終了（コマンド入力に戻す）
         * 
         */
        void seq_end_enemy_turn();
        
        /**
         * @brief メッセージ表示（コマンドを隠してメッセージを出す）
         * 
         * @param text 
         */
        void show_message(const String &text);
    };
} // namespace godot