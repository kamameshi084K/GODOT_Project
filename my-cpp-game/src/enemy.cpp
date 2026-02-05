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
    // --- フィールド移動パラメータ ---
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Enemy::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Enemy::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_detection_range", "p_range"), &Enemy::set_detection_range);
    ClassDB::bind_method(D_METHOD("get_detection_range"), &Enemy::get_detection_range);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detection_range"), "set_detection_range", "get_detection_range");

    ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Enemy::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &Enemy::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

    // --- ★変更: 個別のステータスではなく MonsterData を登録 ---
    ClassDB::bind_method(D_METHOD("set_monster_data", "data"), &Enemy::set_monster_data);
    ClassDB::bind_method(D_METHOD("get_monster_data"), &Enemy::get_monster_data);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "monster_data", PROPERTY_HINT_RESOURCE_TYPE, "MonsterData"), "set_monster_data", "get_monster_data");

    // バトルシーンのパス
    ClassDB::bind_method(D_METHOD("set_battle_scene_path", "p_path"), &Enemy::set_battle_scene_path);
    ClassDB::bind_method(D_METHOD("get_battle_scene_path"), &Enemy::get_battle_scene_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "battle_scene_path", PROPERTY_HINT_FILE, "*.tscn"), "set_battle_scene_path", "get_battle_scene_path");

    // --- ノードパス ---
    ClassDB::bind_method(D_METHOD("set_visual_node_path", "path"), &Enemy::set_visual_node_path);
    ClassDB::bind_method(D_METHOD("get_visual_node_path"), &Enemy::get_visual_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "visual_node_path"), "set_visual_node_path", "get_visual_node_path");

    ClassDB::bind_method(D_METHOD("set_anim_tree_path", "path"), &Enemy::set_anim_tree_path);
    ClassDB::bind_method(D_METHOD("get_anim_tree_path"), &Enemy::get_anim_tree_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree_path"), "set_anim_tree_path", "get_anim_tree_path");

    // 敵ID
    ClassDB::bind_method(D_METHOD("set_enemy_id", "p_id"), &Enemy::set_enemy_id);
    ClassDB::bind_method(D_METHOD("get_enemy_id"), &Enemy::get_enemy_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "enemy_id"), "set_enemy_id", "get_enemy_id");
}

Enemy::Enemy()
{
    speed = 3.0;
    detection_range = 10.0;
    gravity = 9.8;

    battle_scene_path = "res://scenes/battle.tscn";
    visual_node = nullptr;
    anim_tree = nullptr;
}

Enemy::~Enemy()
{
}

void Enemy::_ready()
{
    // ... (既存のロジックそのまま)
    GameManager *gm = GameManager::get_singleton();
    if (gm && !enemy_id.is_empty())
    {
        if (gm->is_enemy_defeated(enemy_id))
        {
            queue_free();
            return; 
        }
    }
    
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

    // ... (移動・追跡ロジックは変更なし) ...
    Vector3 velocity = get_velocity();
    if (!is_on_floor()) velocity.y -= gravity * delta;

    Node* player_node = get_tree()->get_first_node_in_group("player");
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
                
                if (direction.length_squared() > 0.01)
                {
                    Vector3 look_target = player_pos;
                    look_target.y = my_pos.y;
                    look_at(look_target, Vector3(0, 1, 0));
                }
            }
            else
            {
                velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(10.0 * delta));
                velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(10.0 * delta));
            }
        }
    }

    if (anim_tree)
    {
        double h_speed = Vector3(velocity.x, 0, velocity.z).length();
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
            // --- バトル開始処理 ---
            GameManager *gm = GameManager::get_singleton();

            if (gm)
            {
                Node3D *player_3d = Object::cast_to<Node3D>(body);
                if (player_3d)
                {
                    gm->set_last_player_position(player_3d->get_global_position());
                    gm->set_is_returning_from_battle(true);
                }

                if (!enemy_id.is_empty())
                {
                    gm->set_current_enemy_id(enemy_id);
                }

                // ★変更: 個別の数値ではなく、データそのものを渡す
                if (monster_data.is_valid())
                {
                    // GameManagerに追加した set_next_enemy_data を呼ぶ
                    gm->set_next_enemy_data(monster_data);
                }
                else
                {
                    UtilityFunctions::print("Error: Enemy has no MonsterData assigned!");
                }

                gm->set_last_scene_path(get_tree()->get_current_scene()->get_scene_file_path());
            }

            UtilityFunctions::print("Encounter! Battle Start!");

            if (!battle_scene_path.is_empty())
            {
                get_tree()->call_deferred("change_scene_to_file", battle_scene_path);
            }
            return;
        }
    }
}

// --- セッター・ゲッター ---

// ★追加
void Enemy::set_monster_data(const Ref<MonsterData>& data) { monster_data = data; }
Ref<MonsterData> Enemy::get_monster_data() const { return monster_data; }


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

void Enemy::set_enemy_id(const String &p_id) { enemy_id = p_id; }
String Enemy::get_enemy_id() const { return enemy_id; }