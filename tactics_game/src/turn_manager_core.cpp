#include "turn_manager_core.hpp"

using namespace godot;

void TurnManagerCore::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("register_unit", "unit"), &TurnManagerCore::register_unit);
    ClassDB::bind_method(D_METHOD("clear_units"), &TurnManagerCore::clear_units);
    ClassDB::bind_method(D_METHOD("start_match"), &TurnManagerCore::start_match);
    ClassDB::bind_method(D_METHOD("get_current_unit"), &TurnManagerCore::get_current_unit);
    ClassDB::bind_method(D_METHOD("get_current_unit_index"), &TurnManagerCore::get_current_unit_index);
    ClassDB::bind_method(D_METHOD("get_turn_count"), &TurnManagerCore::get_turn_count);
    ClassDB::bind_method(D_METHOD("can_perform_main_action"), &TurnManagerCore::can_perform_main_action);
    ClassDB::bind_method(D_METHOD("consume_main_action"), &TurnManagerCore::consume_main_action);
    ClassDB::bind_method(D_METHOD("next_turn"), &TurnManagerCore::next_turn);
    ClassDB::bind_method(D_METHOD("remove_dead_unit", "unit"), &TurnManagerCore::remove_dead_unit);

    ADD_SIGNAL(MethodInfo("turn_changed", PropertyInfo(Variant::OBJECT, "active_unit", PROPERTY_HINT_RESOURCE_TYPE, "UnitCore"), PropertyInfo(Variant::INT, "turn_count")));
    ADD_SIGNAL(MethodInfo("match_ended", PropertyInfo(Variant::OBJECT, "winner_unit", PROPERTY_HINT_RESOURCE_TYPE, "UnitCore")));
}

TurnManagerCore::TurnManagerCore() {}
TurnManagerCore::~TurnManagerCore() {}

void TurnManagerCore::register_unit(const Ref<UnitCore> &unit)
{
    if (unit.is_valid())
    {
        active_units.append(unit);
    }
}

void TurnManagerCore::clear_units()
{
    active_units.clear();
    current_unit_index = 0;
    turn_count = 1;
}

void TurnManagerCore::start_match()
{
    if (active_units.is_empty()) return;
    current_unit_index = 0;
    turn_count = 1;
    has_used_main_action = false;
    
    Ref<UnitCore> unit = get_current_unit();
    if (unit.is_valid())
    {
        unit->on_turn_start();
        emit_signal("turn_changed", unit, turn_count);
    }
}

Ref<UnitCore> TurnManagerCore::get_current_unit() const
{
    if (active_units.is_empty() || current_unit_index >= active_units.size())
    {
        return Ref<UnitCore>();
    }
    return active_units[current_unit_index];
}

void TurnManagerCore::consume_main_action()
{
    has_used_main_action = true;
}

void TurnManagerCore::next_turn()
{
    if (active_units.is_empty()) return;

    Ref<UnitCore> current = get_current_unit();
    if (current.is_valid())
    {
        current->on_turn_end();
    }

    current_unit_index = (current_unit_index + 1) % active_units.size();
    if (current_unit_index == 0)
    {
        turn_count++;
    }
    has_used_main_action = false;

    Ref<UnitCore> next_unit = get_current_unit();
    if (next_unit.is_valid())
    {
        next_unit->on_turn_start();
        emit_signal("turn_changed", next_unit, turn_count);
    }
}

void TurnManagerCore::remove_dead_unit(const Ref<UnitCore> &unit)
{
    int idx = active_units.find(unit);
    if (idx != -1)
    {
        active_units.remove_at(idx);
        if (current_unit_index >= active_units.size())
        {
            current_unit_index = 0;
        }
        if (active_units.size() == 1)
        {
            emit_signal("match_ended", active_units[0]);
        }
    }
}