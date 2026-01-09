#include "enemy.hpp"
#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/kinematic_collision3d.hpp>

using namespace godot;

void Enemy::_bind_methods()
{
    // --- パラメータ登録 ---
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Enemy::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Enemy::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_detection_range", "p_range"), &Enemy::set_detection_range);
    ClassDB::bind_method(D_METHOD("get_detection_range"), &Enemy::get_detection_range);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detection_range"), "set_detection_range", "get_detection_range");

    ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Enemy::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &Enemy::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

    // バトルシーンのパス（ファイル選択画面が出るように設定）
    ClassDB::bind_method(D_METHOD("set_battle_scene_path", "p_path"), &Enemy::set_battle_scene_path);
    ClassDB::bind_method(D_METHOD("get_battle_scene_path"), &Enemy::get_battle_scene_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "battle_scene_path", PROPERTY_HINT_FILE, "*.tscn"), "set_battle_scene_path", "get_battle_scene_path");

    // --- ノードパス登録 ---
    ClassDB::bind_method(D_METHOD("set_visual_node_path", "path"), &Enemy::set_visual_node_path);
    ClassDB::bind_method(D_METHOD("get_visual_node_path"), &Enemy::get_visual_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "visual_node_path"), "set_visual_node_path", "get_visual_node_path");

    ClassDB::bind_method(D_METHOD("set_anim_tree_path", "path"), &Enemy::set_anim_tree_path);
    ClassDB::bind_method(D_METHOD("get_anim_tree_path"), &Enemy::get_anim_tree_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree_path"), "set_anim_tree_path", "get_anim_tree_path");
}

Enemy::Enemy()
{
    speed = 3.0;
    detection_range = 10.0;
    gravity = 9.8;
    
    // デフォルトのバトルシーンを設定（空なら遷移しないガードを入れる）
    battle_scene_path = "res://battle.tscn";

    visual_node = nullptr;
    anim_tree = nullptr;
}

Enemy::~Enemy()
{
}

void Enemy::_ready()
{
    // ノードの取得
    if (!visual_node_path.is_empty())
    {
        visual_node = get_node<Node3D>(visual_node_path);
    }

    if (!anim_tree_path.is_empty())
    {
        anim_tree = get_node<AnimationTree>(anim_tree_path);
        if (anim_tree)
        {
            anim_tree->set_active(true);
        }
    }
}

void Enemy::_physics_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    Vector3 velocity = get_velocity();

    // 1. 重力
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta;
    }

    // 2. プレイヤー追跡
    Node* player_node = get_tree()->get_first_node_in_group("player");
    bool is_moving = false;

    if (player_node)
    {
        Node3D* player_3d = Object::cast_to<Node3D>(player_node);
        if (player_3d)
        {
            Vector3 my_pos = get_global_position();
            Vector3 player_pos = player_3d->get_global_position();
            
            double dist = my_pos.distance_to(player_pos);
            if (dist < detection_range)
            {
                Vector3 direction = (player_pos - my_pos).normalized();
                direction.y = 0;

                velocity.x = direction.x * speed;
                velocity.z = direction.z * speed;
                is_moving = true;

                // 向きを変える (look_at)
                // ※ look_atは「自分自身」を回転させます
                if (direction.length_squared() > 0.01)
                {
                    Vector3 look_target = player_pos;
                    look_target.y = my_pos.y;
                    look_at(look_target, Vector3(0, 1, 0));
                }
            }
            else
            {
                // 停止（摩擦）
                velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(10.0 * delta));
                velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(10.0 * delta));
            }
        }
    }

    // 3. アニメーション制御 (BlendSpace1D を想定)
    if (anim_tree)
    {
        double h_speed = Vector3(velocity.x, 0, velocity.z).length();
        // Playerと同じく "parameters/Move/blend_position" を動かす
        anim_tree->set("parameters/Move/blend_position", (real_t)h_speed);
    }

    set_velocity(velocity);
    move_and_slide();

    // 4. 衝突判定とバトル開始
    for (int i = 0; i < get_slide_collision_count(); i++)
    {
        Ref<KinematicCollision3D> collision = get_slide_collision(i);
        Object *collider = collision->get_collider();
        Node *body = Object::cast_to<Node>(collider);

        if (body && body->is_in_group("player"))
        {
            GameManager *gm = GameManager::get_singleton();
            if (gm)
            {
                Node3D *player_3d = Object::cast_to<Node3D>(body);
                if (player_3d)
                {
                    gm->set_last_player_position(player_3d->get_global_position());
                    gm->set_is_returning_from_battle(true);
                }
            }

            UtilityFunctions::print("Encounter! Battle Start!");

            // 設定されたバトルシーンへ遷移
            if (!battle_scene_path.is_empty())
            {
                get_tree()->call_deferred("change_scene_to_file", battle_scene_path);
            }
            return;
        }
    }
}

// --- セッター・ゲッター ---

void Enemy::set_speed(double p_speed) { speed = p_speed; }
double Enemy::get_speed() const { return speed; }

void Enemy::set_detection_range(double p_range) { detection_range = p_range; }
double Enemy::get_detection_range() const { return detection_range; }

void Enemy::set_gravity(double p_gravity) { gravity = p_gravity; }
double Enemy::get_gravity() const { return gravity; }

void Enemy::set_battle_scene_path(const String &p_path) { battle_scene_path = p_path; }
String Enemy::get_battle_scene_path() const { return battle_scene_path; }

void Enemy::set_visual_node_path(const NodePath &path) { visual_node_path = path; }
NodePath Enemy::get_visual_node_path() const { return visual_node_path; }

void Enemy::set_anim_tree_path(const NodePath &path) { anim_tree_path = path; }
NodePath Enemy::get_anim_tree_path() const { return anim_tree_path; }