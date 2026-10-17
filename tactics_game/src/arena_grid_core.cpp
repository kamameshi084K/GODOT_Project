#include "arena_grid_core.hpp"

namespace godot
{
    ArenaGridCore::ArenaGridCore()
    {
    }

    ArenaGridCore::~ArenaGridCore()
    {
    }

    void ArenaGridCore::init_tile(const Vector3i &cell, int max_hp)
    {
        tile_durability[cell_to_key(cell)] = max_hp;
    }

    int ArenaGridCore::damage_tile(const Vector3i &cell)
    {
        int64_t key = cell_to_key(cell);
        auto it = tile_durability.find(key);
        if (it == tile_durability.end())
        {
            return -1;
        }

        it->second -= 1;
        int remaining = it->second;
        if (remaining <= 0)
        {
            tile_durability.erase(it);
            hazards.erase(key);
        }
        return remaining;
    }

    int ArenaGridCore::get_tile_state(const Vector3i &cell) const
    {
        int64_t key = cell_to_key(cell);
        auto it = tile_durability.find(key);
        if (it == tile_durability.end())
        {
            return TILE_STATE_DESTROYED;
        }
        return (it->second == 1) ? TILE_STATE_CRACKED : TILE_STATE_NORMAL;
    }

    void ArenaGridCore::repair_tile(const Vector3i &cell, int hp)
    {
        tile_durability[cell_to_key(cell)] = hp;
    }

    PackedVector3Array ArenaGridCore::get_cells_in_radius(
        const Vector3 &center_world,
        float radius,
        const PackedVector3Array &all_tile_positions)
    {
        PackedVector3Array in_range;
        float r_sq = radius * radius;
        Vector3 center_flat(center_world.x, 0.0f, center_world.z);

        for (int i = 0; i < all_tile_positions.size(); ++i)
        {
            Vector3 pos = all_tile_positions[i];
            Vector3 pos_flat(pos.x, 0.0f, pos.z);
            if (pos_flat.distance_squared_to(center_flat) <= r_sq)
            {
                in_range.push_back(pos);
            }
        }
        return in_range;
    }

    void ArenaGridCore::add_hazard(const Vector3i &cell, int stacks, int duration)
    {
        int64_t key = cell_to_key(cell);
        HazardData &h = hazards[key];
        h.poison_stacks = stacks;
        h.remaining_turns = duration;
    }

    bool ArenaGridCore::has_hazard(const Vector3i &cell) const
    {
        return hazards.find(cell_to_key(cell)) != hazards.end();
    }

    int ArenaGridCore::get_hazard_poison_stacks(const Vector3i &cell) const
    {
        auto it = hazards.find(cell_to_key(cell));
        return (it != hazards.end()) ? it->second.poison_stacks : 0;
    }

    TypedArray<Vector3i> ArenaGridCore::tick_hazards()
    {
        TypedArray<Vector3i> expired;
        for (auto it = hazards.begin(); it != hazards.end();)
        {
            it->second.remaining_turns--;
            if (it->second.remaining_turns <= 0)
            {
                expired.push_back(key_to_cell(it->first));
                it = hazards.erase(it);
            }
            else
            {
                ++it;
            }
        }
        return expired;
    }

    void ArenaGridCore::_bind_methods()
    {
        ClassDB::bind_method(D_METHOD("init_tile", "cell", "max_hp"), &ArenaGridCore::init_tile, DEFVAL(2));
        ClassDB::bind_method(D_METHOD("damage_tile", "cell"), &ArenaGridCore::damage_tile);
        ClassDB::bind_method(D_METHOD("get_tile_state", "cell"), &ArenaGridCore::get_tile_state);
        ClassDB::bind_method(D_METHOD("repair_tile", "cell", "hp"), &ArenaGridCore::repair_tile, DEFVAL(2));

        ClassDB::bind_method(D_METHOD("get_cells_in_radius", "center_world", "radius", "all_tile_positions"), &ArenaGridCore::get_cells_in_radius);

        ClassDB::bind_method(D_METHOD("add_hazard", "cell", "stacks", "duration"), &ArenaGridCore::add_hazard);
        ClassDB::bind_method(D_METHOD("has_hazard", "cell"), &ArenaGridCore::has_hazard);
        ClassDB::bind_method(D_METHOD("get_hazard_poison_stacks", "cell"), &ArenaGridCore::get_hazard_poison_stacks);
        ClassDB::bind_method(D_METHOD("tick_hazards"), &ArenaGridCore::tick_hazards);

        BIND_ENUM_CONSTANT(TILE_STATE_NORMAL);
        BIND_ENUM_CONSTANT(TILE_STATE_CRACKED);
        BIND_ENUM_CONSTANT(TILE_STATE_DESTROYED);
    }
} // namespace godot