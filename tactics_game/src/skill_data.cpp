#include "skill_data.hpp"
#include <algorithm>

using namespace godot;

void SkillData::_bind_methods()
{
    // ゲッター / セッターのバインド
    ClassDB::bind_method(D_METHOD("set_skill_name", "name"), &SkillData::set_skill_name);
    ClassDB::bind_method(D_METHOD("get_skill_name"), &SkillData::get_skill_name);
    ClassDB::bind_method(D_METHOD("set_description", "desc"), &SkillData::set_description);
    ClassDB::bind_method(D_METHOD("get_description"), &SkillData::get_description);
    ClassDB::bind_method(D_METHOD("set_action_type", "type"), &SkillData::set_action_type);
    ClassDB::bind_method(D_METHOD("get_action_type"), &SkillData::get_action_type);
    ClassDB::bind_method(D_METHOD("set_target_type", "type"), &SkillData::set_target_type);
    ClassDB::bind_method(D_METHOD("get_target_type"), &SkillData::get_target_type);

    ClassDB::bind_method(D_METHOD("set_range", "range"), &SkillData::set_range);
    ClassDB::bind_method(D_METHOD("get_range"), &SkillData::get_range);
    ClassDB::bind_method(D_METHOD("set_area_radius", "radius"), &SkillData::set_area_radius);
    ClassDB::bind_method(D_METHOD("get_area_radius"), &SkillData::get_area_radius);

    ClassDB::bind_method(D_METHOD("set_base_damage", "damage"), &SkillData::set_base_damage);
    ClassDB::bind_method(D_METHOD("get_base_damage"), &SkillData::get_base_damage);
    ClassDB::bind_method(D_METHOD("set_self_damage", "damage"), &SkillData::set_self_damage);
    ClassDB::bind_method(D_METHOD("get_self_damage"), &SkillData::get_self_damage);
    ClassDB::bind_method(D_METHOD("set_shield_amount", "amount"), &SkillData::set_shield_amount);
    ClassDB::bind_method(D_METHOD("get_shield_amount"), &SkillData::get_shield_amount);
    ClassDB::bind_method(D_METHOD("set_heal_amount", "amount"), &SkillData::set_heal_amount);
    ClassDB::bind_method(D_METHOD("get_heal_amount"), &SkillData::get_heal_amount);

    ClassDB::bind_method(D_METHOD("set_knockback_force", "force"), &SkillData::set_knockback_force);
    ClassDB::bind_method(D_METHOD("get_knockback_force"), &SkillData::get_knockback_force);
    ClassDB::bind_method(D_METHOD("set_breaks_floor", "breaks"), &SkillData::set_breaks_floor);
    ClassDB::bind_method(D_METHOD("get_breaks_floor"), &SkillData::get_breaks_floor);

    ClassDB::bind_method(D_METHOD("set_poison_stacks", "stacks"), &SkillData::set_poison_stacks);
    ClassDB::bind_method(D_METHOD("get_poison_stacks"), &SkillData::get_poison_stacks);
    ClassDB::bind_method(D_METHOD("set_root_turns", "turns"), &SkillData::set_root_turns);
    ClassDB::bind_method(D_METHOD("get_root_turns"), &SkillData::get_root_turns);
    ClassDB::bind_method(D_METHOD("set_attack_buff_multiplier", "mult"), &SkillData::set_attack_buff_multiplier);
    ClassDB::bind_method(D_METHOD("get_attack_buff_multiplier"), &SkillData::get_attack_buff_multiplier);

    ClassDB::bind_method(D_METHOD("set_max_cooldown", "cd"), &SkillData::set_max_cooldown);
    ClassDB::bind_method(D_METHOD("get_max_cooldown"), &SkillData::get_max_cooldown);
    ClassDB::bind_method(D_METHOD("get_current_cooldown"), &SkillData::get_current_cooldown);
    ClassDB::bind_method(D_METHOD("set_max_uses", "uses"), &SkillData::set_max_uses);
    ClassDB::bind_method(D_METHOD("get_max_uses"), &SkillData::get_max_uses);
    ClassDB::bind_method(D_METHOD("get_remaining_uses"), &SkillData::get_remaining_uses);

    ClassDB::bind_method(D_METHOD("is_available"), &SkillData::is_available);
    ClassDB::bind_method(D_METHOD("trigger_use"), &SkillData::trigger_use);
    ClassDB::bind_method(D_METHOD("on_turn_start"), &SkillData::on_turn_start);

    // --- プロパティ公開（インスペクター用） ---
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "skill_name"), "set_skill_name", "get_skill_name");
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "description", PROPERTY_HINT_MULTILINE_TEXT), "set_description", "get_description");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "action_type", PROPERTY_HINT_ENUM, "Main Action,Quick Action"), "set_action_type", "get_action_type");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "target_type", PROPERTY_HINT_ENUM, "Self,Single Enemy,Single Ally,Direction Line,Circle Area,Ground Cell"), "set_target_type", "get_target_type");
    
    // 射程・範囲
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "range"), "set_range", "get_range");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "area_radius"), "set_area_radius", "get_area_radius");
    
    // ダメージ・回復・シールド
    ADD_PROPERTY(PropertyInfo(Variant::INT, "base_damage"), "set_base_damage", "get_base_damage");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "self_damage"), "set_self_damage", "get_self_damage");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "shield_amount"), "set_shield_amount", "get_shield_amount");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "heal_amount"), "set_heal_amount", "get_heal_amount");
    
    // 物理・ギミック
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "knockback_force"), "set_knockback_force", "get_knockback_force");
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "breaks_floor"), "set_breaks_floor", "get_breaks_floor");
    
    // 状態異常・バフ
    ADD_PROPERTY(PropertyInfo(Variant::INT, "poison_stacks"), "set_poison_stacks", "get_poison_stacks");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "root_turns"), "set_root_turns", "get_root_turns");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "attack_buff_multiplier"), "set_attack_buff_multiplier", "get_attack_buff_multiplier");
    
    // クールダウン・回数制限
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_cooldown"), "set_max_cooldown", "get_max_cooldown");
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_uses"), "set_max_uses", "get_max_uses");

    // 列挙型のバインド
    BIND_ENUM_CONSTANT(ACTION_MAIN);
    BIND_ENUM_CONSTANT(ACTION_QUICK);

    BIND_ENUM_CONSTANT(TARGET_SELF);
    BIND_ENUM_CONSTANT(TARGET_SINGLE_ENEMY);
    BIND_ENUM_CONSTANT(TARGET_SINGLE_ALLY);
    BIND_ENUM_CONSTANT(TARGET_DIRECTION_LINE);
    BIND_ENUM_CONSTANT(TARGET_CIRCLE_AREA);
    BIND_ENUM_CONSTANT(TARGET_GROUND_CELL);
}

SkillData::SkillData() {}
SkillData::~SkillData() {}

bool SkillData::is_available() const
{
    if (current_cooldown > 0) return false;
    if (max_uses != -1 && remaining_uses <= 0) return false;
    return true;
}

void SkillData::trigger_use()
{
    current_cooldown = max_cooldown;
    if (max_uses != -1 && remaining_uses > 0)
    {
        remaining_uses--;
    }
}

void SkillData::on_turn_start()
{
    if (current_cooldown > 0)
    {
        current_cooldown--;
    }
}