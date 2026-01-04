#pragma once

#include <godot_cpp/classes/character_body3d.hpp>

namespace godot
{
    class Enemy : public CharacterBody3D
    {
        GDCLASS(Enemy, CharacterBody3D)
    private:
        double speed;
        double detection_range; // この距離に入ったら気づく
        double gravity;

    protected:
        static void _bind_methods();

    public:
        Enemy();
        ~Enemy();

        // キャラクターの動作を制御するために_physics_processをオーバーライド
        // virtualとは、基底クラスで仮想関数として宣言されていることを示します
        virtual void _physics_process(double delta) override;
    };
} // namespace godot
