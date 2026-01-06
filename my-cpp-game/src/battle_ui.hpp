#pragma onece

#include <godot_cpp/classes/control.hpp>

namespace godot
{
    class BattleUI : public Control
    {
        GDCLASS(BattleUI, Control)

    protected:
        static void _bind_methods();

    public:
        BattleUI();
        ~BattleUI();

        // 画面が表示された瞬間に呼ばれる
        virtual void _ready() override;

        // ボタンが押されたら呼ばれる関数
        void _on_win_button_pressed();
    };
} // namespace godot