#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_player.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/animation_node_state_machine_playback.hpp>
#include <godot_cpp/classes/spring_arm3d.hpp>
#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input_event.hpp> // 入力イベント用
#include <godot_cpp/classes/ray_cast3d.hpp>  // RayCast用

namespace godot
{
    class Player : public CharacterBody3D
    {
        GDCLASS(Player, CharacterBody3D)

    private:
        // 移動パラメータ
        double speed;
        double jump_velocity;
        double gravity;
        double camera_sensitivity;
        double acceleration;
        double friction;

        int max_hp;
        int attack_power;
        int defense_power;
        int level;
        int current_exp;
        int exp_to_next_level;

        RayCast3D* interaction_ray;

        // ノードパス（エディタで設定用）
        NodePath visual_node_path;
        NodePath camera_arm_path;
        NodePath anim_tree_path;
        NodePath hitbox_path;

        // 実際のノードへのポインタ（_readyで取得してキャッシュする）
        Node3D* visual_node;
        SpringArm3D* camera_arm;
        AnimationTree* anim_tree;
        AnimationNodeStateMachinePlayback* state_machine;
        Area3D* hitbox;

    protected:
        static void _bind_methods();

    public:
        Player();
        ~Player();

        virtual void _ready() override;
        /**
         * @brief 物理フレームごとの処理
         * 
         * @param delta 
         * @note なんでoverrideするのかというと、CharacterBody3Dの_move_and_slide()を使いたいから。
         */
        virtual void _physics_process(double delta) override;
        /**
         * @brief 入力イベント処理
         * 
         * @param event 
         * @note 入力イベントを直接処理したい場合のためにoverrideしておく。
         */
        virtual void _input(const Ref<InputEvent>& event) override;

        // セッター・ゲッター（エディタ設定用）
        void set_speed(double p_speed);
        double get_speed() const;

        void set_jump_velocity(double p_velocity);
        double get_jump_velocity() const;

        void set_gravity(double p_gravity);
        double get_gravity() const;

        void set_camera_sensitivity(double p_sensitivity);
        double get_camera_sensitivity() const;

        void set_acceleration(double p_accel);
        double get_acceleration() const;

        void set_friction(double p_friction);
        double get_friction() const;

        void set_visual_node_path(const NodePath &path);
        NodePath get_visual_node_path() const;

        void set_camera_arm_path(const NodePath &path);
        NodePath get_camera_arm_path() const;

        void set_anim_tree_path(const NodePath &path);
        NodePath get_anim_tree_path() const;

        void set_hitbox_path(const NodePath &path);
        NodePath get_hitbox_path() const;

        void set_max_hp(int val); int get_max_hp() const;
        void set_attack_power(int val); int get_attack_power() const;
        void set_defense_power(int val); int get_defense_power() const;
        
        void set_level(int val); int get_level() const;
        void set_current_exp(int val); int get_current_exp() const;
        void set_exp_to_next_level(int val); int get_exp_to_next_level() const;
    };
}