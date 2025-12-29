#ifndef GDEXAMPLE_H
#define GDEXAMPLE_H

#include <godot_cpp/classes/character_body2d.hpp>

namespace godot 
{

    // クラスの継承元を変更
    class GDExample : public CharacterBody2D
    {
        GDCLASS(GDExample, CharacterBody2D)

        private:
            double time_passed;
            double speed;
            // セッター・ゲッターの宣言
            void set_speed(double p_speed);
            double get_speed() const;

        protected:
            static void _bind_methods();

        public:
            GDExample();
            ~GDExample();
            virtual void _ready() override;
            // 物理演算をする場合は _process ではなく _physics_process を使うのが定石です
            virtual void _physics_process(double delta) override;
    };
}

#endif