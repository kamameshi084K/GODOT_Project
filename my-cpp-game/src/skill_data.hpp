#pragma once

#include <godot_cpp/classes/resource.hpp>

namespace godot
{
    // じゃんけんの手のタイプ
    enum HandType
    {
        HAND_ROCK = 0,
        HAND_SCISSORS = 1,
        HAND_PAPER = 2
    };

    // 攻撃の種類（物理か魔法かなど。特性でダメージ変動させるため）
    enum AttackType
    {
        ATTACK_PHYSICAL = 0, // 物理
        ATTACK_SPECIAL = 1   // 特殊（魔法）
    };

    class SkillData : public Resource
    {
        GDCLASS(SkillData, Resource)

    private:
        String skill_name;
        int hand_type;   // HandType を int で保存（インスペクター用）
        int attack_type; // AttackType を int で保存
        int power;       // 技の威力
        String animation_name; // 再生するアニメーション名（例: "Punch", "Magic"）
        bool is_physical;      // 相手の目の前まで移動するかどうか（true=移動する, false=その場で撃つ）

    protected:
        static void _bind_methods();

    public:
        SkillData();
        ~SkillData();

        // Setter / Getter
        /**
         * @brief 技の名前を設定・取得
         * 
         * @param name 技の名前
         */
        void set_skill_name(const String& name);
        // 技の名前を取得
        String get_skill_name() const;

        /**
         * @brief じゃんけんの手のタイプを設定・取得
         * 
         * @param type HandType のいずれか
         */
        void set_hand_type(int type);
        // じゃんけんの手のタイプを取得
        int get_hand_type() const;

        /**
         * @brief 攻撃の種類を設定・取得
         * 
         * @param type AttackType のいずれか
         */
        void set_attack_type(int type);
        // 攻撃の種類を取得
        int get_attack_type() const;

        /**
         * @brief 技の威力を設定・取得
         * 
         * @param value 威力の値
         */
        void set_power(int value);
        // 技の威力を取得
        int get_power() const;

        /**
         * @brief 再生するアニメーション名を設定・取得
         * 
         * @param name アニメーション名
         */
        void set_animation_name(const String& name);
        // 再生するアニメーション名を取得
        String get_animation_name() const;

        /**
         * @brief 再生するアニメーション名を設定・取得
         * 
         * @param name アニメーション名
         */
        void set_is_physical(bool is_phys);
        // 技が物理攻撃かどうかを取得
        bool get_is_physical() const;
    };
}