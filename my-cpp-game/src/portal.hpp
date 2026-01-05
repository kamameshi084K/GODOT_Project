#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/node.hpp>

namespace godot
{
    class Portal : public Area3D
    {
        GDCLASS(Portal, Area3D)

    private:
        String next_scene_path; // 次のシーンのファイルパス（例: "res://dungeon.tscn"）

    protected:
        static void _bind_methods();

    public:
        Portal();
        ~Portal();

        virtual void _ready() override;
        
        // シグナルを受け取る関数
        void _on_body_entered(Node* body);

        // プロパティのセッター・ゲッター
        void set_next_scene_path(const String &p_path);
        String get_next_scene_path() const;
    };
} // namespace godot