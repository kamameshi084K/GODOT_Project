#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/templates/vector.hpp>
#include "unit_core.hpp"

namespace godot
{

    class TurnManagerCore : public RefCounted
    {
        GDCLASS(TurnManagerCore, RefCounted);

    public:
        enum TurnPhase
        {
            PHASE_START,
            PHASE_ACTION,
            PHASE_END
        };

    private:
        TypedArray<UnitCore> active_units;
        int current_unit_index = 0;
        int turn_count = 1;
        bool has_used_main_action = false; // 通常行動は1ターンに1回（クイックは除外）
        TurnPhase current_phase = PHASE_START;

    protected:
        static void _bind_methods();

    public:
        TurnManagerCore();
        ~TurnManagerCore();

        void register_unit(const Ref<UnitCore> &unit);
        void clear_units();
        void start_match();
        
        Ref<UnitCore> get_current_unit() const;
        int get_current_unit_index() const { return current_unit_index; }
        int get_turn_count() const { return turn_count; }

        bool can_perform_main_action() const { return !has_used_main_action; }
        void consume_main_action();
        
        void next_turn();
        void remove_dead_unit(const Ref<UnitCore> &unit);
    };

} // namespace godot

/**
 * @brief GDScriptでの使用を可能にするため、TurnPhaseのVariant列挙型キャストを追加。
 * @param TurnManagerCore::TurnPhase 列挙型をVariantとして扱えるようにする。
 */
VARIANT_ENUM_CAST(TurnManagerCore::TurnPhase);