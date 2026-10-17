#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <unordered_map>

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/packed_vector3_array.hpp>
#include <godot_cpp/variant/typed_array.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/vector3i.hpp>
#include <unordered_map>

namespace godot
{
    class ArenaGridCore : public RefCounted
    {
        GDCLASS(ArenaGridCore, RefCounted);

    public:
        enum TileState
        {
            TILE_STATE_NORMAL = 0,
            TILE_STATE_CRACKED = 1,
            TILE_STATE_DESTROYED = 2
        };

        struct HazardData
        {
            int poison_stacks = 0;
            int remaining_turns = 0;
        };

    private:
        std::unordered_map<int64_t, int> tile_durability;
        std::unordered_map<int64_t, HazardData> hazards;

        int64_t cell_to_key(const Vector3i &cell) const
        {
            return ((int64_t)cell.x << 32) ^ ((int64_t)cell.z & 0xFFFFFFFF);
        }

        Vector3i key_to_cell(int64_t key) const
        {
            int32_t x = (int32_t)(key >> 32);
            int32_t z = (int32_t)(key & 0xFFFFFFFF);
            return Vector3i(x, 0, z);
        }

    protected:
        static void _bind_methods();

    public:
        ArenaGridCore();
        ~ArenaGridCore();

        // タイル耐久度・状態管理
        void init_tile(const Vector3i &cell, int max_hp = 2);
        int damage_tile(const Vector3i &cell);
        int get_tile_state(const Vector3i &cell) const;
        void repair_tile(const Vector3i &cell, int hp = 2);

        // 幾何・範囲計算
        PackedVector3Array get_cells_in_radius(
            const Vector3 &center_world,
            float radius,
            const PackedVector3Array &all_tile_positions
        );

        // ハザード（毒床）管理
        void add_hazard(const Vector3i &cell, int stacks, int duration);
        bool has_hazard(const Vector3i &cell) const;
        int get_hazard_poison_stacks(const Vector3i &cell) const;
        TypedArray<Vector3i> tick_hazards();
    };
} // namespace godot

VARIANT_ENUM_CAST(ArenaGridCore::TileState);