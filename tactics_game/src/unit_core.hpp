#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>

#include "skill_data.hpp"

namespace godot
{
    class UnitCore : public RefCounted
    {
        GDCLASS(UnitCore, RefCounted);

    private:
        // プライベートメンバー変数や関数をここに追加できます
        String unit_name = "Unit"; // ユニットの名前
        int max_hp = 100; // ユニットの最大HP
        int current_hp = 100; // ユニットの現在のHP
        int shield = 0; // ユニットのシールド値

        float max_move_distance = 10.0f; // ユニットの最大移動距離
        float remaining_move_distance = 10.0f; // ユニットの残り移動距離

        int poison_stacks = 0; // ユニットの毒スタック数
        float attack_multiplier = 1.0f; // 攻撃力の倍率（デフォルトは1.0）
        bool is_rooted = false; // ユニットが根付いているかどうかのフラグ

        TypedArray<SkillData> skills;

    protected:
    /**
     * @brief 
     * 
     */
        static void _bind_methods();

    public:
        /**
         * @brief これは、ユニットのコア機能を管理するクラスです。HP、シールド、移動距離、毒スタックなどの基本的なステータスを管理し、ターン開始時や終了時の処理も行います。
         * 
         */
        UnitCore();

        /**
         * @brief デストラクタ。必要に応じてリソースの解放やクリーンアップ処理を行います。
         * 
         */
        ~UnitCore();

        /**
         * @brief ユニットにダメージを適用します。シールドがある場合は、まずシールドからダメージを吸収し、残りのダメージがあればHPに適用されます。HPが0になった場合は死亡処理を行います。
         * 
         * @param amount ダメージの量
         */
        void apply_damage(int amount);

        /**
         * @brief ユニットにシールドを追加します。シールドはダメージを吸収するためのバリアとして機能します。
         * 
         * @param amount 追加するシールドの量
         */
        void add_shield(int amount);

        /**
         * @brief ユニットに毒スタックを追加します。毒スタックはターンごとにHPを減少させる効果を持ちます。
         * 
         * @param stacks 追加する毒スタックの数
         */
        void add_poison(int stacks);

        /**
         * @brief ユニットの残り移動距離を消費します。移動距離が不足している場合は、消費できる距離だけを減少させます。
         * 
         * @param distance 消費する移動距離
         * @return true 
         * @return false 
         */
        bool consume_move_distance(float distance);
        
        /**
         * @brief ターン開始時に実行される処理
         */
        void on_turn_start();
        /**
         * @brief ターン終了時に実行される処理
         */
        void on_turn_end();

        void add_skill(const Ref<SkillData> &skill)
        {
            if (skill.is_valid()) skills.append(skill);
        }

        TypedArray<SkillData> get_skills() const { return skills; }
        Ref<SkillData> get_skill(int index) const
        {
            if (index >= 0 && index < skills.size()) return skills[index];
            return Ref<SkillData>();
        }

        // ゲッター / セッター
        int get_current_hp() const { return current_hp; }
        int get_shield() const { return shield; }
        float get_remaining_move_distance() const { return remaining_move_distance; }
        void set_rooted(bool rooted) { is_rooted = rooted; }
        bool get_is_rooted() const { return is_rooted; }
    };
}// namespace godot