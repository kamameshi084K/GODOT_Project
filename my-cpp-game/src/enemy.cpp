#include "enemy.hpp"
#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/kinematic_collision3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp> // 確率計算用

using namespace godot;

void Enemy::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Enemy::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Enemy::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_detection_range", "p_range"), &Enemy::set_detection_range);
    ClassDB::bind_method(D_METHOD("get_detection_range"), &Enemy::get_detection_range);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "detection_range"), "set_detection_range", "get_detection_range");

    ClassDB::bind_method(D_METHOD("set_gravity", "p_gravity"), &Enemy::set_gravity);
    ClassDB::bind_method(D_METHOD("get_gravity"), &Enemy::get_gravity);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "gravity"), "set_gravity", "get_gravity");

    ClassDB::bind_method(D_METHOD("set_monster_data", "data"), &Enemy::set_monster_data);
    ClassDB::bind_method(D_METHOD("get_monster_data"), &Enemy::get_monster_data);
    ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "monster_data", PROPERTY_HINT_RESOURCE_TYPE, "MonsterData"), "set_monster_data", "get_monster_data");

    ClassDB::bind_method(D_METHOD("set_visual_node_path", "path"), &Enemy::set_visual_node_path);
    ClassDB::bind_method(D_METHOD("get_visual_node_path"), &Enemy::get_visual_node_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "visual_node_path"), "set_visual_node_path", "get_visual_node_path");

    ClassDB::bind_method(D_METHOD("set_anim_tree_path", "path"), &Enemy::set_anim_tree_path);
    ClassDB::bind_method(D_METHOD("get_anim_tree_path"), &Enemy::get_anim_tree_path);
    ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "anim_tree_path"), "set_anim_tree_path", "get_anim_tree_path");

    // メソッド登録
    ClassDB::bind_method(D_METHOD("take_damage", "amount"), &Enemy::take_damage);
    ClassDB::bind_method(D_METHOD("hit_by_ball"), &Enemy::hit_by_ball);
}

Enemy::Enemy()
{
    speed = 3.0;
    detection_range = 10.0;
    gravity = 9.8;
    visual_node = nullptr;
    anim_tree = nullptr;
    
    max_hp = 10;
    current_hp = 10;
    hp_label = nullptr;
}

Enemy::~Enemy() {}

void Enemy::_ready()
{
    if (!visual_node_path.is_empty()) visual_node = get_node<Node3D>(visual_node_path);

    if (!anim_tree_path.is_empty()) {
        anim_tree = get_node<AnimationTree>(anim_tree_path);
        if (anim_tree) anim_tree->set_active(true);
    }
    
    // ★追加: HP表示用ラベルの作成
    hp_label = memnew(Label3D);
    add_child(hp_label);
    hp_label->set_position(Vector3(0, 1.5, 0)); // 頭上
    hp_label->set_billboard_mode(BaseMaterial3D::BILLBOARD_ENABLED);
    hp_label->set_font_size(32);
    hp_label->set_modulate(Color(1, 0, 0)); // 赤文字
    
    // データの反映
    if (monster_data.is_valid()) {
        max_hp = monster_data->get_max_hp();
        current_hp = max_hp;
    }
    update_ui();
}

void Enemy::_physics_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint()) return;

    // --- 移動ロジック (既存のまま) ---
    Vector3 velocity = get_velocity();
    if (!is_on_floor()) velocity.y -= gravity * delta;

    Node* player_node = get_tree()->get_first_node_in_group("player");
    if (player_node) {
        Node3D* player_3d = Object::cast_to<Node3D>(player_node);
        if (player_3d) {
            Vector3 my_pos = get_global_position();
            Vector3 player_pos = player_3d->get_global_position();
            double dist = my_pos.distance_to(player_pos);
            
            if (dist < detection_range) {
                Vector3 direction = (player_pos - my_pos).normalized();
                direction.y = 0;
                velocity.x = direction.x * speed;
                velocity.z = direction.z * speed;
                
                if (direction.length_squared() > 0.01) {
                    Vector3 look_target = player_pos;
                    look_target.y = my_pos.y;
                    look_at(look_target, Vector3(0, 1, 0));
                }
            } else {
                velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(10.0 * delta));
                velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(10.0 * delta));
            }
        }
    }

    if (anim_tree) {
        double h_speed = Vector3(velocity.x, 0, velocity.z).length();
        anim_tree->set("parameters/Move/blend_position", (real_t)h_speed);
    }

    set_velocity(velocity);
    move_and_slide();
    
    // ★削除: 衝突時のバトル画面遷移コードは削除しました
}

void Enemy::update_ui() {
    if (hp_label) {
        hp_label->set_text(String::num_int64(current_hp) + " / " + String::num_int64(max_hp));
    }
}

void Enemy::take_damage(int amount) {
    current_hp -= amount;
    if (current_hp <= 0) {
        current_hp = 0;
        UtilityFunctions::print("Enemy defeated! (Failed to capture)");
        queue_free(); // 倒してしまったら消滅
    } else {
        UtilityFunctions::print("Ouch! HP: ", current_hp);
        // ノックバック処理などを入れても良い
    }
    update_ui();
}

void Enemy::hit_by_ball() {
    // 捕獲判定
    float hp_ratio = (float)current_hp / (float)max_hp;
    // HPが減るほど捕まりやすい (20% 〜 100%)
    float capture_chance = 0.2f + (0.8f * (1.0f - hp_ratio));
    
    Ref<RandomNumberGenerator> rng;
    rng.instantiate();
    rng->randomize();
    
    if (rng->randf() < capture_chance) {
        UtilityFunctions::print("Capture Success!");
        GameManager* gm = GameManager::get_singleton();
        if (gm && monster_data.is_valid()) {
            Ref<MonsterData> new_data = monster_data->duplicate(true);
            gm->add_monster(new_data);
        }
        queue_free(); // 消滅
    } else {
        UtilityFunctions::print("Capture Failed!");
        // ボールが弾かれるエフェクトなど
    }
}

// セッター・ゲッター (省略された部分は既存と同じ)
void Enemy::set_monster_data(const Ref<MonsterData>& data) { monster_data = data; }
Ref<MonsterData> Enemy::get_monster_data() const { return monster_data; }
void Enemy::set_speed(double p_speed) { speed = p_speed; }
double Enemy::get_speed() const { return speed; }
void Enemy::set_detection_range(double p_range) { detection_range = p_range; }
double Enemy::get_detection_range() const { return detection_range; }
void Enemy::set_gravity(double p_gravity) { gravity = p_gravity; }
double Enemy::get_gravity() const { return gravity; }
void Enemy::set_visual_node_path(const NodePath &path) { visual_node_path = path; }
NodePath Enemy::get_visual_node_path() const { return visual_node_path; }
void Enemy::set_anim_tree_path(const NodePath &path) { anim_tree_path = path; }
NodePath Enemy::get_anim_tree_path() const { return anim_tree_path; }