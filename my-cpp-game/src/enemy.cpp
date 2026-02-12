#include "enemy.hpp"
#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/kinematic_collision3d.hpp>
#include <godot_cpp/classes/random_number_generator.hpp> // 確率計算用
#include <godot_cpp/classes/animation_node_one_shot.hpp>

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
    is_dying = false;
    
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
    
    // HP表示用ラベルの作成
    hp_label = memnew(Label3D);
    add_child(hp_label);
    float label_height = 1.5; // 計算できなかった時のデフォルト値
    if (visual_node)
    {
        // visual_node の中にある「メッシュ（見た目）」をすべて探す
        // find_children は再帰的に（孫ノードまで）探してくれます
        TypedArray<Node> visuals = visual_node->find_children("*", "VisualInstance3D", true, false);
        
        float max_height = 0.0;
        
        for (int i = 0; i < visuals.size(); i++)
        {
            VisualInstance3D* vis = Object::cast_to<VisualInstance3D>(visuals[i]);
            if (vis)
            {
                // AABB（境界ボックス）を取得して、そのYサイズ（高さ）を見る
                AABB box = vis->get_aabb();
                if (box.size.y > max_height)
                {
                    max_height = box.size.y;
                }
            }
        }
        
        // もし高さが取得できたら採用する（小さすぎる場合は無視）
        if (max_height > 0.5)
        {
            label_height = max_height;
        }
    }
    // 計算した高さ + 少し隙間(0.5m) を空けて配置
    hp_label->set_position(Vector3(0, label_height + 0.5, 0));
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

    if (is_dying)
    {
        set_velocity(Vector3(0, 0, 0));
        move_and_slide();

        if (anim_tree)
        {
            // Dieアニメが終わったかチェック (activeがfalseになったら終了)
            bool is_die_anim_playing = anim_tree->get("parameters/Die/active");
            if (!is_die_anim_playing)
            {
                queue_free(); // アニメが終わったのでさようなら
            }
        }
        else
        {
            queue_free(); // アニメツリーがない場合は即消し
        }
        return; // これ以上何もしない
    }

    // ダメージ（Hit）アニメ中も動かない
    if (anim_tree)
    {
        bool is_hit_anim_playing = anim_tree->get("parameters/Hit/active");
        if (is_hit_anim_playing)
        {
            set_velocity(Vector3(0, 0, 0));
            move_and_slide();
            return;
        }
    }

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
        anim_tree->set("parameters/StateMachine/Move/blend_position", (real_t)h_speed);
    }

    set_velocity(velocity);
    move_and_slide();
    
    //削除: 衝突時のバトル画面遷移コードは削除しました
}

void Enemy::update_ui() {
    if (hp_label) {
        hp_label->set_text(String::num_int64(current_hp) + " / " + String::num_int64(max_hp));
    }
}

void Enemy::take_damage(int amount) {
    if (is_dying) return; // 死んでる最中はダメージ無効

    current_hp -= amount;
    
    if (current_hp <= 0) {
        current_hp = 0;
        UtilityFunctions::print("Enemy Defeated!");
        
        is_dying = true; // 死亡フラグON
        
        // 死亡アニメ再生
        if (anim_tree) {
            anim_tree->set("parameters/Die/request", (int)AnimationNodeOneShot::ONE_SHOT_REQUEST_FIRE);
        }
        
        // ここで queue_free() はしない！（アニメ終了待ちのため）
        
    } else {
        UtilityFunctions::print("Ouch! HP: ", current_hp);
        
        // ダメージアニメ再生
        if (anim_tree) {
            anim_tree->set("parameters/Hit/request", (int)AnimationNodeOneShot::ONE_SHOT_REQUEST_FIRE);
        }
    }
    update_ui();
}

void Enemy::hit_by_ball()
{
    // 1. 捕獲率の計算（HPが減るほど捕まえやすい仕様は維持）
    // HP満タンなら20%、瀕死ならほぼ100%に近い確率になります
    float hp_ratio = (float)current_hp / (float)max_hp;
    float capture_chance = 0.2f + (0.8f * (1.0f - hp_ratio));
    
    Ref<RandomNumberGenerator> rng;
    rng.instantiate();
    rng->randomize();
    
    // 2. 捕獲判定
    if (rng->randf() < capture_chance)
    {
        UtilityFunctions::print("Capture Success!");
        
        GameManager* gm = GameManager::get_singleton();
        if (gm && monster_data.is_valid())
        {
            // データを完全複製して、新しい個体データを作成
            Ref<MonsterData> new_monster = monster_data->duplicate(true);
            
            //重要変更点: 捕まえた瞬間に全回復させる
            // (current_hp に max_hp の値をセットする)
            new_monster->set_current_hp(new_monster->get_max_hp());
            
            //重要変更点: まずは「捕獲（倉庫送り）」として処理する
            // 準備フェーズでパーティ編成を行うため、いきなり手持ちには入れない
            gm->add_captured_monster(new_monster);
        }
        
        // 捕獲成功したので、フィールド上の敵オブジェクトを消滅させる
        queue_free(); 
    }
    else
    {
        UtilityFunctions::print("Capture Failed!");
        // ※必要であれば、ここでボールが弾かれる音やエフェクトを追加できます
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