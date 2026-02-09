#include "player.hpp"
#include "game_manager.hpp"
#include "enemy.hpp"
#include "capture_ball.hpp"

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
    // --- 物理・移動パラメータのバインド ---
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

    // --- ノードパスのバインド ---
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

    // --- リソース（シーン）のバインド ---
    ClassDB::bind_method(D_METHOD("set_capture_ball_scene", "scene"), &Player::set_capture_ball_scene);
    ClassDB::bind_method(D_METHOD("get_capture_ball_scene"), &Player::get_capture_ball_scene);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "capture_ball_scene", PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"), "set_capture_ball_scene", "get_capture_ball_scene");
}

Player::Player()
{
    // 初期パラメータの設定
    speed = 5.0;
    jump_velocity = 4.5;
    gravity = 9.8;
    camera_sensitivity = 0.005;
    acceleration = 40.0;
    friction = 30.0;

    // ポインタの初期化
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
    // NodePathから実体ノードを取得してキャッシュ
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

    // インタラクション用RayCast3Dの動的生成
    interaction_ray = memnew(RayCast3D);
    if (visual_node)
    {
        visual_node->add_child(interaction_ray);
    }
    else
    {
        add_child(interaction_ray);
    }

    // レイキャストのパラメータ設定（正面2mを検知）
    interaction_ray->set_target_position(Vector3(0, 0, 2.0));
    interaction_ray->set_enabled(true);
    interaction_ray->set_position(Vector3(0, 1.0, 0));

    // GameManagerを介したシーン遷移（バトル帰り）の復帰処理
    GameManager *gm = GameManager::get_singleton();
    if (gm)
    {
        if (gm->get_is_returning_from_battle())
        {
            set_global_position(gm->get_last_player_position());
            gm->set_is_returning_from_battle(false);
            UtilityFunctions::print("Welcome back from battle!");
            gm->prepare_battle_stats();
        }
        else
        {
            gm->prepare_battle_stats();
        }
    }
}

void Player::_physics_process(double delta)
{
    // エディタ内での動作防止
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    Input *input = Input::get_singleton();
    Vector3 velocity = get_velocity();

    // 1. 重力計算
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta;
    }

    // 2. カメラ操作（右スティック/マウス等による旋回）
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

    // 3. 移動入力と方向計算
    Vector2 input_dir = input->get_vector("move_left", "move_right", "move_up", "move_down");
    Vector3 direction = Vector3(input_dir.x, 0, input_dir.y);

    if (camera_arm && direction.length() > 0)
    {
        double arm_rotation = camera_arm->get_rotation().y;
        direction = direction.rotated(Vector3(0, 1, 0), arm_rotation);
    }

    // 4. 移動処理（加速・減速）
    if (direction.length() > 0)
    {
        // モデルの向きを移動方向へ滑らかに回転
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
        velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(friction * delta));
        velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(friction * delta));
    }

    // 5. アニメーション制御とアクション
    if (anim_tree)
    {
        // 攻撃アクション
        if (input->is_action_just_pressed("attack"))
        {
            anim_tree->set("parameters/Punch/request", (int)AnimationNodeOneShot::ONE_SHOT_REQUEST_FIRE);
            
            if (hitbox)
            {
                TypedArray<Node3D> bodies = hitbox->get_overlapping_bodies();
                for (int i = 0; i < bodies.size(); i++)
                {
                    Enemy* enemy = Object::cast_to<Enemy>(bodies[i]);
                    if (enemy)
                    {
                        enemy->take_damage(3);
                    }
                }
            }
        }

        // ボール投げアクション
        if (capture_ball_scene.is_valid())
        {
            Node* node = capture_ball_scene->instantiate();
            if (!node) return; // 生成失敗対策

            CaptureBall* ball = Object::cast_to<CaptureBall>(node);
            if (ball) // ここで必ず nullptr チェック！
            {
                // 先にツリーに追加（これにより _ready 等が走る準備ができる）
                get_parent()->add_child(ball);
                
                Vector3 spawn_pos = get_global_position() + Vector3(0, 1.5, 0) - get_transform().basis.get_column(2) * 1.0;
                Vector3 forward = -get_transform().basis.get_column(2);
                
                ball->set_global_position(spawn_pos);
                ball->setup(forward);
            }
            else
            {
                // キャストに失敗した場合はメモリリーク防止のため削除
                UtilityFunctions::print("Error: Instantiated node is not a CaptureBall!");
                node->queue_free();
            }
        }

        // 移動アニメーション（水平速度をパラメータに渡す）
        double h_speed = Vector3(velocity.x, 0, velocity.z).length();
        anim_tree->set("parameters/Move/blend_position", (real_t)h_speed);

        // ステートマシンによる接地・空中状態の遷移
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
    if (Engine::get_singleton()->is_editor_hint())
    {
        return;
    }

    // 決定/インタラクトボタンの入力処理
    if (event->is_action_pressed("ui_accept"))
    {
        if (interaction_ray && interaction_ray->is_colliding())
        {
            Object* collider = interaction_ray->get_collider();
            Node* hit_node = Object::cast_to<Node>(collider);

            if (hit_node && hit_node->has_method("interact"))
            {
                hit_node->call("interact");
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

void Player::set_capture_ball_scene(const Ref<PackedScene>& scene) { capture_ball_scene = scene; }
Ref<PackedScene> Player::get_capture_ball_scene() const { return capture_ball_scene; }