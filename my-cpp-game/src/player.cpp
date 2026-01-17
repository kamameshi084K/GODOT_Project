#include "player.hpp"
#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_event_joypad_button.hpp>
#include <godot_cpp/classes/input_event_joypad_motion.hpp>
#include <godot_cpp/classes/animation_node_one_shot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void Player::_bind_methods()
{
    // --- パラメータの登録 ---
    
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Player::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Player::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_jump_velocity", "p_velocity"), &Player::set_jump_velocity);
    ClassDB::bind_method(D_METHOD("get_jump_velocity"), &Player::get_jump_velocity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "jump_velocity"), "set_jump_velocity", "get_jump_velocity");

    ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Player::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &Player::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

    ClassDB::bind_method(D_METHOD("set_camera_sensitivity", "p_sensitivity"), &Player::set_camera_sensitivity);
    ClassDB::bind_method(D_METHOD("get_camera_sensitivity"), &Player::get_camera_sensitivity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "camera_sensitivity"), "set_camera_sensitivity", "get_camera_sensitivity");

    ClassDB::bind_method(D_METHOD("set_acceleration", "p_accel"), &Player::set_acceleration);
    ClassDB::bind_method(D_METHOD("get_acceleration"), &Player::get_acceleration);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "acceleration"), "set_acceleration", "get_acceleration");

    ClassDB::bind_method(D_METHOD("set_friction", "p_friction"), &Player::set_friction);
    ClassDB::bind_method(D_METHOD("get_friction"), &Player::get_friction);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "friction"), "set_friction", "get_friction");

    // --- ステータスの登録 ---
    ClassDB::bind_method(D_METHOD("set_max_hp", "val"), &Player::set_max_hp);
    ClassDB::bind_method(D_METHOD("get_max_hp"), &Player::get_max_hp);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_max_hp"), "set_max_hp", "get_max_hp");

    ClassDB::bind_method(D_METHOD("set_attack_power", "val"), &Player::set_attack_power);
    ClassDB::bind_method(D_METHOD("get_attack_power"), &Player::get_attack_power);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_attack_power"), "set_attack_power", "get_attack_power");

    ClassDB::bind_method(D_METHOD("set_defense_power", "val"), &Player::set_defense_power);
    ClassDB::bind_method(D_METHOD("get_defense_power"), &Player::get_defense_power);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_defense_power"), "set_defense_power", "get_defense_power");

    ClassDB::bind_method(D_METHOD("set_level", "val"), &Player::set_level);
    ClassDB::bind_method(D_METHOD("get_level"), &Player::get_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_level"), "set_level", "get_level");

    ClassDB::bind_method(D_METHOD("set_current_exp", "val"), &Player::set_current_exp);
    ClassDB::bind_method(D_METHOD("get_current_exp"), &Player::get_current_exp);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_current_exp"), "set_current_exp", "get_current_exp");

    ClassDB::bind_method(D_METHOD("set_exp_to_next_level", "val"), &Player::set_exp_to_next_level);
    ClassDB::bind_method(D_METHOD("get_exp_to_next_level"), &Player::get_exp_to_next_level);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "stats_next_exp"), "set_exp_to_next_level", "get_exp_to_next_level");

    // --- ノードパスの登録 ---

    ClassDB::bind_method(D_METHOD("set_visual_node_path", "path"), &Player::set_visual_node_path);
    ClassDB::bind_method(D_METHOD("get_visual_node_path"), &Player::get_visual_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "visual_node_path"), "set_visual_node_path", "get_visual_node_path");

    ClassDB::bind_method(D_METHOD("set_camera_arm_path", "path"), &Player::set_camera_arm_path);
    ClassDB::bind_method(D_METHOD("get_camera_arm_path"), &Player::get_camera_arm_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "camera_arm_path"), "set_camera_arm_path", "get_camera_arm_path");

    ClassDB::bind_method(D_METHOD("set_anim_tree_path", "path"), &Player::set_anim_tree_path);
    ClassDB::bind_method(D_METHOD("get_anim_tree_path"), &Player::get_anim_tree_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree_path"), "set_anim_tree_path", "get_anim_tree_path");

    ClassDB::bind_method(D_METHOD("set_hitbox_path", "path"), &Player::set_hitbox_path);
    ClassDB::bind_method(D_METHOD("get_hitbox_path"), &Player::get_hitbox_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "hitbox_path"), "set_hitbox_path", "get_hitbox_path");
}

Player::Player()
{
    // デフォルト値
    speed = 5.0;
    jump_velocity = 4.5;
    gravity = 9.8;
    camera_sensitivity = 0.005;
    acceleration = 40.0;
    friction = 30.0;

    max_hp = 20;
    attack_power = 5;
    defense_power = 2;
    level = 1;
    current_exp = 0;
    exp_to_next_level = 50;

    visual_node = nullptr;
    camera_arm = nullptr;
    anim_tree = nullptr;
    state_machine = nullptr;
    hitbox = nullptr;
}

Player::~Player()
{
}

void Player::_ready()
{
    if (!visual_node_path.is_empty())
    {
        visual_node = get_node<Node3D>(visual_node_path);
    }
    
    if (!camera_arm_path.is_empty())
    {
        camera_arm = get_node<SpringArm3D>(camera_arm_path);
    }

    if (!anim_tree_path.is_empty())
    {
        anim_tree = get_node<AnimationTree>(anim_tree_path);
        if (anim_tree)
        {
            anim_tree->set_active(true);
            state_machine = Object::cast_to<AnimationNodeStateMachinePlayback>(anim_tree->get("parameters/playback"));
        }
    }

    if (!hitbox_path.is_empty())
    {
        hitbox = get_node<Area3D>(hitbox_path);
    }

    // RayCast3Dノードの作成と設定
    interaction_ray = memnew(RayCast3D);
    // visual_node（モデル）があるなら、その子にすることで回転に追従させる
    if (visual_node)
    {
        visual_node->add_child(interaction_ray);
    }
    else
    {
        // モデルがない場合は仕方なくPlayer直下に追加
        add_child(interaction_ray);
    }

    // 設定: プレイヤーの目の前 2メートルまで検知
    // (Z軸マイナス方向が正面)
    interaction_ray->set_target_position(Vector3(0, 0, 2.0));
    
    // 重要: デフォルトではオフになっているので有効化する
    interaction_ray->set_enabled(true);
    
    // 足元(0,0,0)ではなく、胸のあたり(高さ1.0)から飛ばす
    interaction_ray->set_position(Vector3(0, 1.0, 0));

    // GameManagerのチェック（バトル帰りかどうか）
    GameManager *gm = GameManager::get_singleton();
    if (gm)
    {
        // バトルから帰ってきたか？
        if (gm->get_is_returning_from_battle())
        {
            // ■ パターンA: バトルから帰還時
            // 位置を復元する
            set_global_position(gm->get_last_player_position());
            gm->set_is_returning_from_battle(false);
            
            UtilityFunctions::print("Welcome back from battle!");
            
            // 【重要】
            // 帰ってきた時は、ステータスを初期化したくない！（レベル1に戻ってしまうから）
            // 本当はここで「今のHP」などをGameManagerから読み込む処理が入ります（次回実装）
        }
        else
        {
            // ■ パターンB: ゲーム開始時（または初回起動）
            // ここで初めてエディタの設定値(20など)をGameManagerに送る！
            
            gm->init_player_stats(max_hp, attack_power, defense_power, level, current_exp, exp_to_next_level);
            
            UtilityFunctions::print("Game Start! Sending Editor Stats to GM. Attack: ", attack_power);
        }
    }
}

void Player::_physics_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    Input *input = Input::get_singleton();
    Vector3 velocity = get_velocity();

    // 1. 重力
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta;
    }

    // 3. カメラ操作
    Vector2 cam_input = input->get_vector("camera_left", "camera_right", "camera_up", "camera_down");
    
    if (camera_arm)
    {
        camera_arm->rotate_y(-cam_input.x * camera_sensitivity);
        
        double current_rotation_x = camera_arm->get_rotation_degrees().x;
        double new_rotation_x = current_rotation_x - (cam_input.y * camera_sensitivity * 50.0);
        new_rotation_x = Math::clamp(new_rotation_x, (double)-90.0, (double)30.0);

        Vector3 rot = camera_arm->get_rotation_degrees();
        rot.x = new_rotation_x;
        camera_arm->set_rotation_degrees(rot);
    }

    // 4. 移動入力
    Vector2 input_dir = input->get_vector("move_left", "move_right", "move_up", "move_down");
    Vector3 direction = Vector3(input_dir.x, 0, input_dir.y);

    if (camera_arm && direction.length() > 0)
    {
        double arm_rotation = camera_arm->get_rotation().y;
        direction = direction.rotated(Vector3(0, 1, 0), arm_rotation);
    }

    if (direction.length() > 0)
    {
        // 向きの更新
        if (visual_node)
        {
            double target_angle = Math::atan2(direction.x, direction.z);
            Vector3 rotation = visual_node->get_rotation();
            rotation.y = Math::lerp_angle(rotation.y, (real_t)target_angle, (real_t)(15.0 * delta));
            visual_node->set_rotation(rotation);
        }

        // 加速
        velocity.x = Math::move_toward(velocity.x, (real_t)(direction.x * speed), (real_t)(acceleration * delta));
        velocity.z = Math::move_toward(velocity.z, (real_t)(direction.z * speed), (real_t)(acceleration * delta));
    }
    else
    {
        // 減速（摩擦）
        velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(friction * delta));
        velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(friction * delta));
    }

    // 5. アニメーションと攻撃
    if (anim_tree)
    {
        // 攻撃入力
        if (input->is_action_just_pressed("attack"))
        {
            anim_tree->set("parameters/Punch/request", (int)AnimationNodeOneShot::ONE_SHOT_REQUEST_FIRE);
            UtilityFunctions::print("Attack!");

            // 捕食ロジック（Hitboxがあれば実行）
            if (hitbox)
            {
                TypedArray<Node3D> bodies = hitbox->get_overlapping_bodies();
                for (int i = 0; i < bodies.size(); i++)
                {
                    Node3D* body = Object::cast_to<Node3D>(bodies[i]);
                    if (body && body->is_in_group("enemy"))
                    {
                        UtilityFunctions::print("Gotcha! Ate the enemy!");
                        body->queue_free();
                    }
                }
            }
        }

        // 移動アニメーション
        double h_speed = Vector3(velocity.x, 0, velocity.z).length();
        anim_tree->set("parameters/Move/blend_position", (real_t)h_speed);

        // ジャンプ/移動の状態遷移
        if (state_machine)
        {
            if (is_on_floor())
            {
                state_machine->travel("Move");
            }
            else
            {
                state_machine->travel("Jump");
            }
        }
    }

    set_velocity(velocity);
    move_and_slide();
}

void Player::_input(const Ref<InputEvent>& event)
{
    // エディタ上での実行を防ぐ
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // "ui_accept" (Enter, Space, Aボタン等) が押されたかチェック
    if (event->is_action_pressed("ui_accept"))
    {
        // レイキャストが存在し、何かに衝突しているか？
        if (interaction_ray && interaction_ray->is_colliding())
        {
            // 当たったオブジェクトを取得
            Object* collider = interaction_ray->get_collider();
            Node* hit_node = Object::cast_to<Node>(collider);

            if (hit_node)
            {
                // デバッグ用: 当たったものの名前を表示
                UtilityFunctions::print("Hit object: ", hit_node->get_name());

                // ここに後で「会話処理」などを追加します
            }
        }
    }
}

// --- セッター・ゲッターの実装 ---

void Player::set_speed(double p_speed) { speed = p_speed; }
double Player::get_speed() const { return speed; }

void Player::set_jump_velocity(double p_velocity) { jump_velocity = p_velocity; }
double Player::get_jump_velocity() const { return jump_velocity; }

void Player::set_gravity(double p_gravity) { gravity = p_gravity; }
double Player::get_gravity() const { return gravity; }

void Player::set_camera_sensitivity(double p_sensitivity) { camera_sensitivity = p_sensitivity; }
double Player::get_camera_sensitivity() const { return camera_sensitivity; }

void Player::set_acceleration(double p_accel) { acceleration = p_accel; }
double Player::get_acceleration() const { return acceleration; }

void Player::set_friction(double p_friction) { friction = p_friction; }
double Player::get_friction() const { return friction; }

void Player::set_visual_node_path(const NodePath &path) { visual_node_path = path; }
NodePath Player::get_visual_node_path() const { return visual_node_path; }

void Player::set_camera_arm_path(const NodePath &path) { camera_arm_path = path; }
NodePath Player::get_camera_arm_path() const { return camera_arm_path; }

void Player::set_anim_tree_path(const NodePath &path) { anim_tree_path = path; }
NodePath Player::get_anim_tree_path() const { return anim_tree_path; }

void Player::set_hitbox_path(const NodePath &path) { hitbox_path = path; }
NodePath Player::get_hitbox_path() const { return hitbox_path; }

void Player::set_max_hp(int val) { max_hp = val; }
int Player::get_max_hp() const { return max_hp; }

void Player::set_attack_power(int val) { attack_power = val; }
int Player::get_attack_power() const { return attack_power; }

void Player::set_defense_power(int val) { defense_power = val; }
int Player::get_defense_power() const { return defense_power; }

void Player::set_level(int val) { level = val; }
int Player::get_level() const { return level; }

void Player::set_current_exp(int val) { current_exp = val; }
int Player::get_current_exp() const { return current_exp; }

void Player::set_exp_to_next_level(int val) { exp_to_next_level = val; }
int Player::get_exp_to_next_level() const { return exp_to_next_level; }