#pragma once

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot 
{

    class Player : public CharacterBody3D
    {
        GDCLASS(Player, CharacterBody3D)

    private:
        double speed;
        double gravity;
        NodePath aim_cursor_path;
        Node3D* aim_cursor_node;

    protected:
        static void _bind_methods();

    public:
        Player();
        ~Player();

        void _ready() override;
        void _physics_process(double delta) override;

        void set_aim_cursor_path(const NodePath& p_path);
        NodePath get_aim_cursor_path() const;
    };
}