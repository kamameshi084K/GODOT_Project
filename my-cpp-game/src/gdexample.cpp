#include "gdexample.h"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void GDExample::_bind_methods() 
{
    // ここに関数やプロパティを登録するとGodotエディタから見えるようになります
}

GDExample::GDExample() 
{
    // コンストラクタ（初期化）
    time_passed = 0.0;
}

GDExample::~GDExample() 
{
    // デストラクタ
}

void GDExample::_process(double delta) 
{
    // 毎フレーム呼ばれる処理
    time_passed += delta;

    // サイン波でゆらゆら動かす
    Vector2 new_position = Vector2(20.0 + (10.0 * sin(time_passed * 10.0)), 10.0 + (10.0 * cos(time_passed * 1.5)));
    
    set_position(new_position);
}