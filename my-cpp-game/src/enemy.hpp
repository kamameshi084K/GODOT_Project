#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp> // アニメーション用
#include <godot_cpp/classes/engine.hpp>

namespace godot
{
    class Enemy : public CharacterBody3D
    {
        GDCLASS(Enemy, CharacterBody3D)

    private:
        // パラメータ
        double speed;
        double detection_range; // 気づく距離
        double gravity;
        
        // 遷移先のバトルシーン（例: "res://battle.tscn"）
        String battle_scene_path; // バトルシーンのパス
        String battler_visual_path; // バトラーのビジュアルシーンのパス
        String enemy_id; // 敵のID

        // ノードパス（エディタ設定用）
        NodePath visual_node_path;
        NodePath anim_tree_path;

        // 実際のノードへのポインタ
        Node3D* visual_node;
        AnimationTree* anim_tree;

        String enemy_name; // 表示名（例: "ゴブリン"）
        int max_hp;        // 戦闘時の体力
        int attack_power;  // 攻撃力
        int defense_power; // 防御力
        int exp_reward; // 倒した時に与える経験値

    protected:
        static void _bind_methods();

    public:
        Enemy();
        ~Enemy();

        virtual void _ready() override;
        virtual void _physics_process(double delta) override;

        // --- セッター・ゲッター ---
        void set_speed(double p_speed);
        double get_speed() const;

        void set_detection_range(double p_range);
        double get_detection_range() const;

        void set_gravity(double p_gravity);
        double get_gravity() const;

        void set_battle_scene_path(const String &p_path);
        String get_battle_scene_path() const;

        void set_visual_node_path(const NodePath &path);
        NodePath get_visual_node_path() const;

        void set_anim_tree_path(const NodePath &path);
        NodePath get_anim_tree_path() const;

        void set_battler_visual_path(const String &p_path);
        String get_battler_visual_path() const;

        void set_enemy_id(const String &p_id);
        String get_enemy_id() const;

        void set_enemy_name(const String &p_name);
        String get_enemy_name() const;

        void set_max_hp(int p_hp);
        int get_max_hp() const;

        void set_attack_power(int p_power);
        int get_attack_power() const;

        void set_defense_power(int p_def);
        int get_defense_power() const;

        void set_exp_reward(int p_exp);
        int get_exp_reward() const;
    };
}