#pragma once

#include "monster_data.hpp" // ★追加

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/engine.hpp>

namespace godot
{
    class Enemy : public CharacterBody3D
    {
        GDCLASS(Enemy, CharacterBody3D)

    private:
        // パラメータ（フィールド上の動き用）
        double speed;
        double detection_range;
        double gravity;
        
        // ★追加: これ1つですべてのデータ（ステータス・バトル時の見た目）を管理
        Ref<MonsterData> monster_data;

        // 遷移先のバトルシーン（例: "res://battle.tscn"）
        String battle_scene_path;
        String enemy_id;

        // フィールド上の見た目用ノードパス
        NodePath visual_node_path;
        NodePath anim_tree_path;

        Node3D* visual_node;
        AnimationTree* anim_tree;

    protected:
        static void _bind_methods();

    public:
        Enemy();
        ~Enemy();

        virtual void _ready() override;
        virtual void _physics_process(double delta) override;

        // --- セッター・ゲッター ---
        
        /**
         * @brief モンスターのデータを設定・取得する
         * 
         * @param data MonsterData リソース
         */
        void set_monster_data(const Ref<MonsterData>& data);
        /**
         * @brief モンスターのデータを取得する
         * 
         * @return Ref<MonsterData> モンスターのデータ
         */
        Ref<MonsterData> get_monster_data() const;

        /**
         * @brief 速度の設定・取得
         * 
         * @param p_speed 速度
         */
        void set_speed(double p_speed);
        /**
         * @brief 速度の取得
         * 
         * @return double 速度
         */
        double get_speed() const;

        /**
         * @brief 発見範囲の設定・取得
         * 
         * @param p_range 発見範囲
         */
        void set_detection_range(double p_range);
        /**
         * @brief 発見範囲の取得
         * 
         * @return double 発見範囲
         */
        double get_detection_range() const;

        /**
         * @brief 重力の設定・取得
         * 
         * @param p_gravity 重力
         */
        void set_gravity(double p_gravity);
        /**
         * @brief 重力の取得
         * 
         * @return double 重力
         */
        double get_gravity() const;

        /**
         * @brief バトルシーンのパスの設定・取得
         * 
         * @param p_path バトルシーンのパス
         */
        void set_battle_scene_path(const String &p_path);
        /**
         * @brief バトルシーンのパスの取得
         * 
         * @return String バトルシーンのパス
         */
        String get_battle_scene_path() const;

        /**
         * @brief フィールド上の見た目ノードパスの設定・取得
         * 
         * @param path 見た目ノードパス
         */
        void set_visual_node_path(const NodePath &path);
        /**
         * @brief フィールド上の見た目ノードパスの取得
         * 
         * @return NodePath 見た目ノードパス
         */
        NodePath get_visual_node_path() const;

        /**
         * @brief アニメーションツリーパスの設定・取得
         * 
         * @param path アニメーションツリーパス
         */
        void set_anim_tree_path(const NodePath &path);
        /**
         * @brief アニメーションツリーパスの取得
         * 
         * @return NodePath アニメーションツリーパス
         */
        NodePath get_anim_tree_path() const;

        /**
         * @brief 敵IDの設定・取得
         * 
         * @param p_id 敵ID
         */
        void set_enemy_id(const String &p_id);
        /**
         * @brief 敵IDの取得
         * 
         * @return String 敵ID
         */
        String get_enemy_id() const;
    };
}