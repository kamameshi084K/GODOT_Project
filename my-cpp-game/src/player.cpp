#include "player.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp> // ボタン用
#include <godot_cpp/classes/input_event_joypad_motion.hpp> // スティック用
#include <godot_cpp/classes/spring_arm3d.hpp> // カメラアーム用
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/animation_library.hpp>
#include <godot_cpp/classes/animation.hpp>

using namespace godot;

void Player::_bind_methods()
{
    // 必要ならここにプロパティ登録などを書きます
}

Player::Player()
{
    speed = 5.0;
    jump_velocity = 4.5;
    // 一般的な重力値 (プロジェクト設定から取得するのが理想ですが、一旦固定値で)
    gravity = 9.8;
    camera_sensitivity = 0.005;
    acceleration = 40.0; // 値が大きいほど、すぐにトップスピードになる
    friction = 30.0;     // 値が大きいほど、すぐに止まる（小さいと氷の上みたいになる）
}

Player::~Player()
{
}

void Player::_ready()
{
    InputMap *map = InputMap::get_singleton();

    // move_right (Dキー) を登録
    if (!map->has_action("move_right"))
    {
        map->add_action("move_right");
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_D);
        map->action_add_event("move_right", key);
    }

    // move_left (Aキー) を登録
    if (!map->has_action("move_left"))
    {
        map->add_action("move_left");
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_A);
        map->action_add_event("move_left", key);
    }

    // move_up (Wキー) を登録
    if (!map->has_action("move_up"))
    {
        map->add_action("move_up");
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_W);
        map->action_add_event("move_up", key);
    }

    // move_down (Sキー) を登録
    if (!map->has_action("move_down"))
    {
        map->add_action("move_down");
        Ref<InputEventKey> key;
        key.instantiate();
        key->set_keycode(Key::KEY_S);
        map->action_add_event("move_down", key);
    }
    
    // 1. 右移動 (左スティックを右に倒した時)
    Ref<InputEventJoypadMotion> joy_right;
    joy_right.instantiate();
    joy_right->set_axis(JoyAxis::JOY_AXIS_LEFT_X); // 左スティックのX軸
    joy_right->set_axis_value(1.0); // プラス方向 (右)
    map->action_add_event("move_right", joy_right);

    // 2. 左移動 (左スティックを左に倒した時)
    Ref<InputEventJoypadMotion> joy_left;
    joy_left.instantiate();
    joy_left->set_axis(JoyAxis::JOY_AXIS_LEFT_X);
    joy_left->set_axis_value(-1.0); // マイナス方向 (左)
    map->action_add_event("move_left", joy_left);

    // 3. 上移動 (左スティックを上に倒した時)
    // 注意: ゲームパッドのY軸は「上がマイナス」なことが多いですが
    // GodotのInput Map上では軸の方向をそのまま登録します
    Ref<InputEventJoypadMotion> joy_up;
    joy_up.instantiate();
    joy_up->set_axis(JoyAxis::JOY_AXIS_LEFT_Y);
    joy_up->set_axis_value(-1.0); // 上 (マイナス)
    map->action_add_event("move_up", joy_up);

    // 4. 下移動 (左スティックを下に倒した時)
    Ref<InputEventJoypadMotion> joy_down;
    joy_down.instantiate();
    joy_down->set_axis(JoyAxis::JOY_AXIS_LEFT_Y);
    joy_down->set_axis_value(1.0); // 下 (プラス)
    map->action_add_event("move_down", joy_down);

    // 1. カメラ右
    if (!map->has_action("camera_right"))
    {
        map->add_action("camera_right");
        Ref<InputEventJoypadMotion> joy;
        joy.instantiate();
        joy->set_axis(JoyAxis::JOY_AXIS_RIGHT_X);
        joy->set_axis_value(1.0);
        map->action_add_event("camera_right", joy);
    }

    // 2. カメラ左
    if (!map->has_action("camera_left"))
    {
        map->add_action("camera_left");
        Ref<InputEventJoypadMotion> joy;
        joy.instantiate();
        joy->set_axis(JoyAxis::JOY_AXIS_RIGHT_X);
        joy->set_axis_value(-1.0);
        map->action_add_event("camera_left", joy);
    }

    // 3. カメラ上
    if (!map->has_action("camera_up"))
    {
        map->add_action("camera_up");
        Ref<InputEventJoypadMotion> joy;
        joy.instantiate();
        joy->set_axis(JoyAxis::JOY_AXIS_RIGHT_Y);
        joy->set_axis_value(-1.0);
        map->action_add_event("camera_up", joy);
    }

    // 4. カメラ下
    if (!map->has_action("camera_down"))
    {
        map->add_action("camera_down");
        Ref<InputEventJoypadMotion> joy;
        joy.instantiate();
        joy->set_axis(JoyAxis::JOY_AXIS_RIGHT_Y);
        joy->set_axis_value(1.0);
        map->action_add_event("camera_down", joy);
    }

    // 5. ジャンプ (コントローラーの一番下のボタン: PSなら×、XboxならA)
    // まだ ui_accept を作っていない場合は追加
    if (!map->has_action("ui_accept"))
    {
        map->add_action("ui_accept");
    }
    // 既存のアクションにイベントを追加
    Ref<InputEventJoypadButton> joy_jump;
    joy_jump.instantiate();
    joy_jump->set_button_index(JoyButton::JOY_BUTTON_A); // 一般的な決定/ジャンプボタン
    map->action_add_event("ui_accept", joy_jump);

    // AnimationPlayerを取得
    // ※ノードパスはあなたの環境に合わせてください ("Idle/AnimationPlayer" など)
    anim_player = Object::cast_to<AnimationPlayer>(get_node_or_null("Idle/AnimationPlayer"));
    
    if (anim_player)
    {
        // ライブラリを取得（なければ作る）
        Ref<AnimationLibrary> lib;
        if (anim_player->has_animation_library(""))
        {
            lib = anim_player->get_animation_library("");
        } else {
            lib.instantiate();
            anim_player->add_animation_library("", lib);
        }

        if (lib.is_valid()) 
        {
            // 1. Run の登録
            Ref<Resource> run_anim = ResourceLoader::get_singleton()->load("res://Run.res");
            if (run_anim.is_valid())
            {
                if (!lib->has_animation("Run"))
                {
                    lib->add_animation("Run", run_anim);
                    UtilityFunctions::print("Success: Run animation added!");
                }
            }
            else
            {
                UtilityFunctions::print("ERROR: Could not load Run.res");
            }

            // 2. Idle の登録 (ここを追加！)
            Ref<Resource> idle_anim = ResourceLoader::get_singleton()->load("res://Idle.res");
            if (idle_anim.is_valid())
            {
                if (!lib->has_animation("Idle"))
                {
                    lib->add_animation("Idle", idle_anim);
                    UtilityFunctions::print("Success: Idle animation added!");
                }
            }
            else
            {
                UtilityFunctions::print("ERROR: Could not load Idle.res. Check file name (case sensitive!)");
            }
        }
    }
    else
    {
        UtilityFunctions::print("ERROR: AnimationPlayer node not found!");
    }
}

void Player::_physics_process(double delta)
{
    Input *input = Input::get_singleton();
    Vector3 velocity = get_velocity();

    // 1. 重力 (地面にいないなら落下させる)
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta;
    }

    // 2. ジャンプ (スペースキー "ui_accept" かつ 地面にいるとき)
    if (input->is_action_just_pressed("ui_accept") && is_on_floor())
    {
        velocity.y = jump_velocity;
    }

    // カメラ操作入力の取得
    Vector2 cam_input = input->get_vector("camera_left", "camera_right", "camera_up", "camera_down");
    // シーン内にある "SpringArm3D" という名前のノードを探して取得する
    SpringArm3D* spring_arm = Object::cast_to<SpringArm3D>(get_node_or_null("SpringArm3D"));

    // カメラアームが存在すれば回転させる
    if (spring_arm)
    {
        // 横回転 (Y軸まわり)
        // マイナスを掛けているのは、右に倒したときに右を向くようにするため
        spring_arm->rotate_y(-cam_input.x * camera_sensitivity);

        // 縦回転 (X軸まわり)
        // 縦回転はグルグル回りすぎるとおかしくなるので、角度制限(Clamp)をかけます
        double current_rotation_x = spring_arm->get_rotation_degrees().x;
        double new_rotation_x = current_rotation_x - (cam_input.y * camera_sensitivity * 50.0); // 縦は少し速めに
        
        // 角度を -90度(真上) 〜 30度(ちょっと下) に制限
        new_rotation_x = Math::clamp(new_rotation_x, (double)-90.0, (double)30.0);

        Vector3 rot = spring_arm->get_rotation_degrees();
        rot.x = new_rotation_x;
        spring_arm->set_rotation_degrees(rot);
    }

    // 3. 移動入力の取得
    // 3Dなので Vector2 で入力を取って、XZ平面の移動量に変換します
    // "move_left", "move_right" などの設定は2Dの時のままでOKです
    // 3. 移動入力の取得
    Vector2 input_dir = input->get_vector("move_left", "move_right", "move_up", "move_down");
    
    // まず、入力だけのベクトルを作る
    Vector3 direction = Vector3(input_dir.x, 0, input_dir.y);
    // 入力ベクトルを正規化 (長さ1に) する
    if (spring_arm && direction.length() > 0)
    {
        // 【重要】入力ベクトルを、SpringArmのY軸回転に合わせて回す！
        // Vector3::UP は (0, 1, 0) つまりY軸のこと
        double arm_rotation = spring_arm->get_rotation().y;
        // Y軸 (0, 1, 0) を基準に回転させる
        direction = direction.rotated(Vector3(0, 1, 0), arm_rotation);
    }

    if (direction.length() > 0)
    {
        // 修正前: get_node_or_null("MeshInstance3D")
        // 修正後: FBXモデルのノード名 "Idle" に変更！
        Node3D* visual_node = Object::cast_to<Node3D>(get_node_or_null("Idle"));
        
        if (visual_node)
        {
            double target_angle = Math::atan2(direction.x, direction.z);
            Vector3 rotation = visual_node->get_rotation();
            rotation.y = Math::lerp_angle(rotation.y, (real_t)target_angle, (real_t)(15.0 * delta));
            visual_node->set_rotation(rotation);
        }
        velocity.x = Math::move_toward(velocity.x, (real_t)(direction.x * speed), (real_t)(acceleration * delta));
        velocity.z = Math::move_toward(velocity.z, (real_t)(direction.z * speed), (real_t)(acceleration * delta));
    }
    else
    {
        // 入力がない時： 「摩擦」を使って、0になるまで徐々に減速する
        velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(friction * delta));
        velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(friction * delta));
    }

    if (anim_player)
    {
        double horizontal_speed = Vector3(velocity.x, 0, velocity.z).length();
        if (horizontal_speed > 0.1) 
        {
            // 動いているなら Run
            if (anim_player->get_current_animation() != "Run")
            {
                anim_player->play("Run", -1, 1.5);
            }
        } 
        else 
        {
            // 止まっているなら Idle
            if (anim_player->get_current_animation() != "Idle")
            {
                anim_player->play("Idle", 0.2);
            }
        }
    }

    // move_and_slide(); の直前に入れる
    if (direction.length() > 0) {
        UtilityFunctions::print("Velocity: ", velocity, " Direction: ", direction);
    }
    
    set_velocity(velocity);
    move_and_slide();
    
}