#pragma once

#include "monster_data.hpp"
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/label3d.hpp>

namespace godot
{
    /**
     * @class Enemy
     * @brief 敵キャラクターの基幹クラス
     */
    class Enemy : public CharacterBody3D
    {
        GDCLASS(Enemy, CharacterBody3D)

    private:
        // 移動速度
        double speed;
        // プレイヤーの検知範囲
        double detection_range;
        // 適用される重力
        double gravity;
        
        // モンスターの基本データを保持するリソース
        Ref<MonsterData> monster_data;

        // 最大体力
        int max_hp;
        // 現在の体力
        int current_hp;
        // 頭上のHP表示用ラベル
        Label3D* hp_label;

        // 見た目（モデル）ノードのパス
        NodePath visual_node_path;
        // アニメーションツリーのパス
        NodePath anim_tree_path;
        // モデルノードへのポインタ
        Node3D* visual_node;
        // アニメーションツリーへのポインタ
        AnimationTree* anim_tree;

    protected:
        // Godotへのメソッド登録用
        static void _bind_methods();

    public:
        // コンストラクタ
        Enemy();
        // デストラクタ
        ~Enemy();

        // ノード準備完了時の処理
        virtual void _ready() override;

        /**
         * @brief 物理演算更新処理
         * @param delta 前フレームからの経過時間
         */
        virtual void _physics_process(double delta) override;

        // --- セッター・ゲッター ---

        /**
         * @brief モンスターデータを設定
         * @param data モンスターデータリソース
         */
        void set_monster_data(const Ref<MonsterData>& data);

        /**
         * @brief モンスターデータを取得
         * @return 設定されているモンスターデータ
         */
        Ref<MonsterData> get_monster_data() const;

        /**
         * @brief 移動速度を設定
         * @param p_speed 速度
         */
        void set_speed(double p_speed);

        /**
         * @brief 移動速度を取得
         * @return 速度
         */
        double get_speed() const;

        /**
         * @brief 検知範囲を設定
         * @param p_range 範囲
         */
        void set_detection_range(double p_range);

        /**
         * @brief 検知範囲を取得
         * @return 範囲
         */
        double get_detection_range() const;

        /**
         * @brief 重力を設定
         * @param p_gravity 重力値
         */
        void set_gravity(double p_gravity);

        /**
         * @brief 重力を取得
         * @return 重力値
         */
        double get_gravity() const;
        
        /**
         * @brief 見た目ノードのパスを設定
         * @param path ノードパス
         */
        void set_visual_node_path(const NodePath &path);

        /**
         * @brief 見た目ノードのパスを取得
         * @return ノードパス
         */
        NodePath get_visual_node_path() const;

        /**
         * @brief アニメーションツリーのパスを設定
         * @param path ノードパス
         */
        void set_anim_tree_path(const NodePath &path);

        /**
         * @brief アニメーションツリーのパスを取得
         * @return ノードパス
         */
        NodePath get_anim_tree_path() const;

        // --- アクション用メソッド ---

        // HP表示の更新
        void update_ui();

        /**
         * @brief ダメージを受ける処理
         * @param amount ダメージ量
         */
        void take_damage(int amount);

        // ボールが当たった時の処理
        void hit_by_ball();
    };
}