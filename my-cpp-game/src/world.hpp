#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/label.hpp>

namespace godot
{
    class World : public Node3D
    {
        GDCLASS(World, Node3D)
    private:
        Label* time_label;
    protected:
        static void _bind_methods() {}
    public:
        World();
        ~World();
        // 初期化処理
        virtual void _ready() override;
        /**
         * @brief 毎フレームの処理
         * 
         * @param delta フレーム間の経過時間（秒）
         */
        virtual void _process(double delta) override;
    };
}