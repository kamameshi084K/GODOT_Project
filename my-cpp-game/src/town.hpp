#pragma once
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/label.hpp>
#include <godot_cpp/classes/button.hpp>

namespace godot
{
    class Town : public Node3D
    {
        GDCLASS(Town, Node3D)
    private:
        Label* time_label;
        Button* ready_button;
    protected:
        static void _bind_methods();
    public:
        Town();
        ~Town();
        // 初期化処理
        virtual void _ready() override;
        /**
         * @brief 毎フレームの処理
         * 
         * @param delta フレーム間の経過時間（秒）
         */
        virtual void _process(double delta) override;
        
        // 準備完了ボタンを押したときの処理
        void _on_ready_button_pressed();
    };
}