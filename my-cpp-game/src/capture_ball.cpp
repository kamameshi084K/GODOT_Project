#include "capture_ball.hpp"
#include "enemy.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>

using namespace godot;

void CaptureBall::_bind_methods()
{
    // シグナル通知やスクリプトからの呼び出し用にメソッドをバインド
    ClassDB::bind_method(D_METHOD("_on_body_entered", "body"), &CaptureBall::_on_body_entered);
    ClassDB::bind_method(D_METHOD("setup", "dir"), &CaptureBall::setup);
}

CaptureBall::CaptureBall()
{
    // 初期パラメータの設定
    speed = 15.0f;
    life_time = 3.0f;
    direction = Vector3(0, 0, 0);
}

CaptureBall::~CaptureBall()
{
}

void CaptureBall::setup(Vector3 dir)
{
    // 飛んでいく方向を正規化して設定
    direction = dir.normalized();
}

void CaptureBall::_ready()
{
    // エディタ上での実行（ツールモード）による誤作動を防止
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // 衝突検知の有効化
    set_monitorable(true);
    set_monitoring(true);
    
    // 衝突イベント（シグナル）を自身のハンドラに接続
    if (!is_connected("body_entered", Callable(this, "_on_body_entered")))
    {
        connect("body_entered", Callable(this, "_on_body_entered"));
    }
}

void CaptureBall::_process(double delta)
{
    // エディタ上では移動や消滅処理を行わない
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // 直進移動
    set_global_position(get_global_position() + direction * speed * delta);

    // 寿命管理：一定時間経過で自身を削除
    life_time -= delta;
    if (life_time <= 0)
    {
        queue_free();
    }
}

void CaptureBall::_on_body_entered(Node3D* body)
{
    // 衝突した対象をEnemyクラスにキャストして確認
    Enemy* enemy = Object::cast_to<Enemy>(body);
    
    if (enemy)
    {
        // 敵に命中した場合の専用処理を呼び出し
        enemy->hit_by_ball();
        
        // 命中したためボールを削除
        queue_free(); 
    }
    else
    {
        // 敵以外（地形など）に接触した場合もボールを削除
        queue_free();
    }
}