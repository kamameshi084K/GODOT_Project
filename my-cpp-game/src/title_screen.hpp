#pragma once

#include <godot_cpp/classes/control.hpp>

namespace godot
{
    class TitleScreen : public Control
    {
        GDCLASS(TitleScreen, Control)

    protected:
        static void _bind_methods();

    public:
        // コンストラクタ・デストラクタ
        TitleScreen();
        ~TitleScreen();

        // ボタンが押された時の関数
        /**
         * @brief スタートボタンが押された時の処理
         * 
         */
        void _on_start_button_pressed();
        /**
         * @brief 終了ボタンが押された時の処理
         * 
         */
        void _on_quit_button_pressed();
    };
}