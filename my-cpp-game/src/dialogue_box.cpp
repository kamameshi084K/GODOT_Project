#include "dialogue_box.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/viewport.hpp>

using namespace godot;

void DialogueBox::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("show_message", "message"), &DialogueBox::show_message);
    ClassDB::bind_method(D_METHOD("close_message"), &DialogueBox::close_message);

    // プロパティ登録
    ClassDB::bind_method(D_METHOD("set_label_path", "path"), &DialogueBox::set_label_path);
    ClassDB::bind_method(D_METHOD("get_label_path"), &DialogueBox::get_label_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "label_path"), "set_label_path", "get_label_path");

    ClassDB::bind_method(D_METHOD("set_panel_path", "path"), &DialogueBox::set_panel_path);
    ClassDB::bind_method(D_METHOD("get_panel_path"), &DialogueBox::get_panel_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "panel_path"), "set_panel_path", "get_panel_path");
}

DialogueBox::DialogueBox()
{
    text_label = nullptr;
    background_panel = nullptr;
    is_active = false;
}

DialogueBox::~DialogueBox()
{
}

void DialogueBox::_ready()
{
    // ノード取得
    if (!label_path.is_empty())
    {
        text_label = get_node<Label>(label_path);
    }
    if (!panel_path.is_empty())
    {
        background_panel = get_node<Control>(panel_path);
    }

    // 最初は非表示にする
    close_message();

    // 【重要】自分を「dialogue_ui」グループに登録する
    // これでNPCが「誰がメッセージボックスなの？」と探せるようになります
    add_to_group("dialogue_ui");
}

void DialogueBox::_input(const Ref<InputEvent>& event)
{
    // ウィンドウが開いている時に Enterキー が押されたら閉じる
    if (is_active && event->is_action_pressed("ui_accept"))
    {
        close_message();
        
        // 入力をここで食い止めて、プレイヤーがジャンプしたりしないようにする
        get_viewport()->set_input_as_handled();
    }
}

void DialogueBox::show_message(const String& message)
{
    if (text_label)
    {
        text_label->set_text(message);
    }
    
    if (background_panel)
    {
        background_panel->set_visible(true);
    }

    is_active = true;
}

void DialogueBox::close_message()
{
    if (background_panel)
    {
        background_panel->set_visible(false);
    }
    is_active = false;
}

// セッター・ゲッター
void DialogueBox::set_label_path(const NodePath& path) { label_path = path; }
NodePath DialogueBox::get_label_path() const { return label_path; }

void DialogueBox::set_panel_path(const NodePath& path) { panel_path = path; }
NodePath DialogueBox::get_panel_path() const { return panel_path; }