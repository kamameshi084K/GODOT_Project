#include "unit_core.hpp"
#include <algorithm>

using namespace godot;

void UnitCore::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("apply_damage", "amount"), &UnitCore::apply_damage);
    ClassDB::bind_method(D_METHOD("add_shield", "amount"), &UnitCore::add_shield);
    ClassDB::bind_method(D_METHOD("add_poison", "stacks"), &UnitCore::add_poison);
    ClassDB::bind_method(D_METHOD("consume_move_distance", "distance"), &UnitCore::consume_move_distance);
    ClassDB::bind_method(D_METHOD("on_turn_start"), &UnitCore::on_turn_start);
    ClassDB::bind_method(D_METHOD("on_turn_end"), &UnitCore::on_turn_end);

    ClassDB::bind_method(D_METHOD("get_current_hp"), &UnitCore::get_current_hp);
    ClassDB::bind_method(D_METHOD("get_shield"), &UnitCore::get_shield);
    ClassDB::bind_method(D_METHOD("get_remaining_move_distance"), &UnitCore::get_remaining_move_distance);
    ClassDB::bind_method(D_METHOD("set_rooted", "rooted"), &UnitCore::set_rooted);
    ClassDB::bind_method(D_METHOD("get_is_rooted"), &UnitCore::get_is_rooted);

    ClassDB::bind_method(D_METHOD("add_skill", "skill"), &UnitCore::add_skill);
    ClassDB::bind_method(D_METHOD("get_skills"), &UnitCore::get_skills);
    ClassDB::bind_method(D_METHOD("get_skill", "index"), &UnitCore::get_skill);

    ADD_SIGNAL(MethodInfo("hp_changed", PropertyInfo(Variant::INT, "current_hp"), PropertyInfo(Variant::INT, "max_hp")));
    ADD_SIGNAL(MethodInfo("shield_changed", PropertyInfo(Variant::INT, "shield")));
    ADD_SIGNAL(MethodInfo("died"));
}

UnitCore::UnitCore()
{
    // コンストラクタで初期化処理を行う場合はここにかく
}

UnitCore::~UnitCore()
{
    // デストラクタでクリーンアップ処理を行う場合もここ
}

void UnitCore::apply_damage(int amount)
{
    int damage = amount;
    if (shield > 0)
    {
        if (shield >= damage)
        {
            shield -= damage;
            damage = 0;
        }
        else
        {
            damage -= shield;
            shield = 0;
        }
        emit_signal("shield_changed", shield);
    }

    if (damage > 0)
    {
        current_hp = std::max(0, current_hp - damage);
        emit_signal("hp_changed", current_hp, max_hp);
    }

    if (current_hp == 0)
    {
        emit_signal("died");
    }
}

void UnitCore::add_shield(int amount)
{
    shield += amount;
    emit_signal("shield_changed", shield);
}

void UnitCore::add_poison(int stacks)
{
    poison_stacks += stacks;
}

bool UnitCore::consume_move_distance(float distance)
{
    if (is_rooted || remaining_move_distance <= 0.0f)
    {
        return false;
    }
    remaining_move_distance = std::max(0.0f, remaining_move_distance - distance);
    return true;
}

void UnitCore::on_turn_start()
{
    remaining_move_distance = max_move_distance;

    // スキルのクールダウン減少
    for (int i = 0; i < skills.size(); i++)
    {
        Ref<SkillData> skill = skills[i];
        if (skill.is_valid())
        {
            skill->on_turn_start();
        }
    }

    if (poison_stacks > 0)
    {
        apply_damage(poison_stacks);
    }
}

void UnitCore::on_turn_end()
{
    is_rooted = false;
}

