#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/core/class_db.hpp>

namespace godot
{
    class SkillData : public Resource
    {
        GDCLASS(SkillData, Resource);

    public:
        enum ActionType
        {
            ACTION_MAIN = 0,  // 通常行動（手番を消費）
            ACTION_QUICK = 1  // クイック行動（手番を消費しない）
        };

        enum TargetType
        {
            TARGET_SELF = 0,            // 自身対象（バフ/シールドなど）
            TARGET_SINGLE_ENEMY = 1,     // 単体敵指定
            TARGET_SINGLE_ALLY = 2,      // 単体味方指定（回復など）
            TARGET_DIRECTION_LINE = 3,   // 直線貫通方向指定
            TARGET_CIRCLE_AREA = 4,      // 自身中心・指定地点の範囲
            TARGET_GROUND_CELL = 5       // 地面セル指定（毒沼・魔法陣召喚）
        };

    private:
        String skill_name = "New Skill";
        String description = "";
        ActionType action_type = ACTION_MAIN;
        TargetType target_type = TARGET_SINGLE_ENEMY;

        // 射程・範囲
        float range = 3.0f;
        float area_radius = 1.0f;

        // 威力・基本効果
        int base_damage = 0;
        int self_damage = 0;
        int shield_amount = 0;
        int heal_amount = 0;
        float knockback_force = 0.0f;
        bool breaks_floor = false;

        // 状態異常・バフ
        int poison_stacks = 0;
        int root_turns = 0;
        float attack_buff_multiplier = 1.0f; // 次ターンの火力倍率（例: 2.0で2倍）

        // クールダウン・回数制限
        int max_cooldown = 0;
        int current_cooldown = 0;
        int max_uses = -1; // -1で無制限
        int remaining_uses = -1;

    protected:
        static void _bind_methods();

    public:
        SkillData();
        ~SkillData();

        bool is_available() const;
        void trigger_use();
        void on_turn_start();

        // ゲッター / セッター
        void set_skill_name(const String &name) { skill_name = name; }
        String get_skill_name() const { return skill_name; }

        void set_description(const String &desc) { description = desc; }
        String get_description() const { return description; }

        void set_action_type(ActionType type) { action_type = type; }
        ActionType get_action_type() const { return action_type; }

        void set_target_type(TargetType type) { target_type = type; }
        TargetType get_target_type() const { return target_type; }

        void set_range(float r) { range = r; }
        float get_range() const { return range; }

        void set_area_radius(float r) { area_radius = r; }
        float get_area_radius() const { return area_radius; }

        void set_base_damage(int dmg) { base_damage = dmg; }
        int get_base_damage() const { return base_damage; }

        void set_self_damage(int dmg) { self_damage = dmg; }
        int get_self_damage() const { return self_damage; }

        void set_shield_amount(int shield) { shield_amount = shield; }
        int get_shield_amount() const { return shield_amount; }

        void set_heal_amount(int heal) { heal_amount = heal; }
        int get_heal_amount() const { return heal_amount; }

        void set_knockback_force(float force) { knockback_force = force; }
        float get_knockback_force() const { return knockback_force; }

        void set_breaks_floor(bool breaks) { breaks_floor = breaks; }
        bool get_breaks_floor() const { return breaks_floor; }

        void set_poison_stacks(int stacks) { poison_stacks = stacks; }
        int get_poison_stacks() const { return poison_stacks; }

        void set_root_turns(int turns) { root_turns = turns; }
        int get_root_turns() const { return root_turns; }

        void set_attack_buff_multiplier(float mult) { attack_buff_multiplier = mult; }
        float get_attack_buff_multiplier() const { return attack_buff_multiplier; }

        void set_max_cooldown(int cd) { max_cooldown = cd; }
        int get_max_cooldown() const { return max_cooldown; }

        int get_current_cooldown() const { return current_cooldown; }

        void set_max_uses(int uses) { max_uses = uses; remaining_uses = uses; }
        int get_max_uses() const { return max_uses; }
        int get_remaining_uses() const { return remaining_uses; }
    };
} // namespace godot

VARIANT_ENUM_CAST(SkillData::ActionType);
VARIANT_ENUM_CAST(SkillData::TargetType);