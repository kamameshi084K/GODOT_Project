#include "town.hpp"
#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Town::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_ready_button_pressed"), &Town::_on_ready_button_pressed);
}

Town::Town()
{
    time_label = nullptr;
    ready_button = nullptr;
}
Town::~Town() {}

void Town::_ready()
{
    // UIノードの取得
    Node* label_node = find_child("TimeLabel", true, false);
    if (label_node) 
    {
        time_label = Object::cast_to<Label>(label_node);
    }
    Node* btn_node = find_child("ReadyButton", true, false);
    if (btn_node)
    {
        ready_button = Object::cast_to<Button>(btn_node);
        ready_button->connect("pressed", Callable(this, "_on_ready_button_pressed"));
    }
}

void Town::_process(double delta)
{
    GameManager* gm = GameManager::get_singleton();
    if (gm && time_label)
    {
        float time = gm->get_time_remaining();
        if (time <= 0)
        {
            time_label->set_text("Time's Up! Please Ready up.");
        }
        else
        {
            time_label->set_text("Time Left: " + String::num((int)time));
        }
    }
}

void Town::_on_ready_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        gm->set_player_ready(); // サーバーに「準備OK」と報告
        UtilityFunctions::print("Ready sent!");
        
        // 連打防止のためボタンを無効化
        if (ready_button)
        {
            ready_button->set_disabled(true);
        }
    }
}