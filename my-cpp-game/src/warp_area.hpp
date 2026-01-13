#pragma once

#include <godot_cpp/classes/area3d.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot
{
    class WarpArea : public Area3D
    {
        GDCLASS(WarpArea, Area3D)

    private:
        String target_scene_path; // 行き先のシーンファイルパス
        Vector3 target_position;  // 行き先での出現座標

    protected:
        static void _bind_methods();

    public:
        WarpArea();
        ~WarpArea();

        virtual void _ready() override;

        // シグナルを受け取る関数
        void _on_body_entered(Node* body);

        // プロパティのセッター・ゲッター
        void set_target_scene_path(const String &p_path);
        String get_target_scene_path() const;

        void set_target_position(const Vector3 &p_pos);
        Vector3 get_target_position() const;
    };
}