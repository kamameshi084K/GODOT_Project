#pragma once

#include "capture_ball.hpp"

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/animation_node_state_machine_playback.hpp>
#include <godot_cpp/classes/spring_arm3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/ray_cast3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot
{
    /**
     * @class Player
     * @brief プレイヤーキャラクターの制御クラス
     */
    class Player : public CharacterBody3D
    {
        GDCLASS(Player, CharacterBody3D)

    private:
        // --- 移動・物理パラメータ ---
        // 基本移動速度
        double speed;
        // ジャンプの初速度
        double jump_velocity;
        // 適用される重力
        double gravity;
        // カメラの回転感度
        double camera_sensitivity;
        // 移動時の加速度
        double acceleration;
        // 停止時の摩擦力（減速度）
        double friction;

        // --- ノードパス（エディタ設定用） ---
        // モデルの外見ノードのパス
        NodePath visual_node_path;
        // カメラアーム（SpringArm3D）のパス
        NodePath camera_arm_path;
        // AnimationTreeのパス
        NodePath anim_tree_path;
        // 攻撃判定用Area3Dのパス
        NodePath hitbox_path;

        // --- キャッシュされたノードポインタ ---
        // インタラクション判定用のレイキャスト
        RayCast3D* interaction_ray;
        // 外見用ノード
        Node3D* visual_node;
        // カメラアームノード
        SpringArm3D* camera_arm;
        // アニメーション制御ツリー
        AnimationTree* anim_tree;
        // アニメーションの状態管理
        AnimationNodeStateMachinePlayback* state_machine;
        // 攻撃判定エリア
        Area3D* hitbox;

        // --- リソース類 ---
        // 投擲するボールのシーンリソース
        Ref<PackedScene> capture_ball_scene;

    protected:
        // Godotへのメソッド登録用
        static void _bind_methods();

    public:
        // コンストラクタ
        Player();
        // デストラクタ
        ~Player();

        // ノード準備完了時の初期化
        virtual void _ready() override;

        /**
         * @brief 物理演算フレームごとの処理
         * @param delta 前フレームからの経過時間
         */
        virtual void _physics_process(double delta) override;

        /**
         * @brief 入力イベントの受信処理
         * @param event 入力イベント（マウス移動、キー入力等）
         */
        virtual void _input(const Ref<InputEvent>& event) override;

        // --- 移動パラメータのセッター・ゲッター ---

        /**
         * @brief 移動速度を設定
         * @param p_speed 速度
         */
        void set_speed(double p_speed);
        double get_speed() const;

        /**
         * @brief ジャンプ速度を設定
         * @param p_velocity ジャンプ初速
         */
        void set_jump_velocity(double p_velocity);
        double get_jump_velocity() const;

        /**
         * @brief 重力を設定
         * @param p_gravity 重力
         */
        void set_gravity(double p_gravity);
        double get_gravity() const;

        /**
         * @brief カメラ感度を設定
         * @param p_sensitivity 感度
         */
        void set_camera_sensitivity(double p_sensitivity);
        double get_camera_sensitivity() const;

        /**
         * @brief 加速度を設定
         * @param p_accel 加速度
         */
        void set_acceleration(double p_accel);
        double get_acceleration() const;

        /**
         * @brief 摩擦力を設定
         * @param p_friction 摩擦力
         */
        void set_friction(double p_friction);
        double get_friction() const;

        // --- ノード・リソース設定用のセッター・ゲッター ---

        /**
         * @brief 外見ノードのパスを設定
         * @param path ノードパス
         */
        void set_visual_node_path(const NodePath &path);
        NodePath get_visual_node_path() const;

        /**
         * @brief カメラアームのパスを設定
         * @param path ノードパス
         */
        void set_camera_arm_path(const NodePath &path);
        NodePath get_camera_arm_path() const;

        /**
         * @brief AnimationTreeのパスを設定
         * @param path ノードパス
         */
        void set_anim_tree_path(const NodePath &path);
        NodePath get_anim_tree_path() const;

        /**
         * @brief ヒットボックスのパスを設定
         * @param path ノードパス
         */
        void set_hitbox_path(const NodePath &path);
        NodePath get_hitbox_path() const;

        /**
         * @brief キャプチャーボールのシーンを設定
         * @param scene シーンリソース
         */
        void set_capture_ball_scene(const Ref<PackedScene>& scene);
        Ref<PackedScene> get_capture_ball_scene() const;
    };
}