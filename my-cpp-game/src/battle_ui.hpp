#pragma once

#include <godot_cpp/classes/control.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/progress_bar.hpp>

namespace godot
{
    class BattleUI : public Control
    {
        // Godotのマクロ
        GDCLASS(BattleUI, Control)

    private:
        int player_hp; // プレイヤーのHP
        int max_player_hp; // プレイヤーの最大HP
        int enemy_hp; // 敵のHP
        NodePath enemy_anim_path; // エディタで設定する「敵のアニメーションプレイヤーへのパス」
        AnimationPlayer *enemy_anim; // 実際のポインタ

        NodePath player_anim_path; // エディタで設定する「プレイヤーのアニメーションプレイヤーへのパス」
        AnimationPlayer *player_anim; // 実際のポインタ

        ProgressBar *player_hp_bar; // HP表示ラベルへのポインタ

    protected:
        static void _bind_methods();

    public:
        BattleUI();
        ~BattleUI();

        /**
         * @brief 画面が表示された瞬間に呼ばれる
         */
        virtual void _ready() override;

        /**
         * @brief　「攻撃ボタン」が押されたときに呼ばれる
         * @note エディタのシグナル接続画面から接続される想定
         */
        void _on_attack_button_pressed();
        
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
    };
} // namespace godot