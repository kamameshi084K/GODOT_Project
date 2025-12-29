#include "gdexample.h"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>

#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>

using namespace godot;

void GDExample::_bind_methods()
{
    // 1. セッターとゲッターをGodotに見えるように登録
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &GDExample::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &GDExample::get_speed);

    // 2. プロパティとして登録 (インスペクタに表示させる)
    // ADD_PROPERTY(PropertyInfo(型, "表示名"), "セッター", "ゲッター");
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");
}

GDExample::GDExample() 
{
    // コンストラクタ（初期化）
    time_passed = 0.0;
    speed = 400.0; // 移動速度の初期値
}

GDExample::~GDExample() 
{
    // デストラクタ
}

void GDExample::_ready() {
    InputMap *map = InputMap::get_singleton();

    // 右移動 (move_right) を登録
    if (!map->has_action("move_right")) {
        map->add_action("move_right");
        
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_D); // Dキー
        map->action_add_event("move_right", key);
    }

    // 左移動 (move_left) を登録
    if (!map->has_action("move_left")) {
        map->add_action("move_left");
        
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_A); // Aキー
        map->action_add_event("move_left", key);
    }

    // 上移動 (move_up) を登録
    if (!map->has_action("move_up")) {
        map->add_action("move_up");
        
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_W); // Wキー
        map->action_add_event("move_up", key);
    }

    // 下移動 (move_down) を登録
    if (!map->has_action("move_down")) {
        map->add_action("move_down");
        
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_S); // Sキー
        map->action_add_event("move_down", key);
    }
}

void GDExample::_physics_process(double delta)
{
    Input *input = Input::get_singleton();
    Vector2 input_direction = Vector2(0, 0);

    // 入力を受け取る (スタイル修正済み)
    if (input->is_action_pressed("move_right"))
    {
        input_direction.x += 1.0;
    }
    if (input->is_action_pressed("move_left"))
    {
        input_direction.x -= 1.0;
    }
    if (input->is_action_pressed("move_down"))
    {
        input_direction.y += 1.0;
    }
    if (input->is_action_pressed("move_up"))
    {
        input_direction.y -= 1.0;
    }

    // 移動処理
    if (input_direction.length() > 0)
    {
        // 正規化してスピードを掛ける
        Vector2 new_velocity = input_direction.normalized() * speed;
        
        // CharacterBody2D は set_velocity で速度をセットします
        set_velocity(new_velocity);
        
        // move_and_slide() が「衝突判定」と「移動」を同時にやってくれます
        move_and_slide();
    }
    else
    {
        // 入力がないときは止まる
        set_velocity(Vector2(0, 0));
        move_and_slide();
    }
}
// ファイルの最後に追加
void GDExample::set_speed(double p_speed) 
{
    speed = p_speed;
}

double GDExample::get_speed() const
{
    return speed;
}