#include "enemy_spawner.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <cmath>
#include <cstdlib>
#include <godot_cpp/classes/multiplayer_spawner.hpp>

using namespace godot;

void EnemySpawner::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_enemy_scenes", "scenes"), &EnemySpawner::set_enemy_scenes);
    ClassDB::bind_method(D_METHOD("get_enemy_scenes"), &EnemySpawner::get_enemy_scenes);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "enemy_scenes", PROPERTY_HINT_ARRAY_TYPE, "PackedScene"), "set_enemy_scenes", "get_enemy_scenes");

    // ★追加: ウェイトプロパティの登録
    ClassDB::bind_method(D_METHOD("set_enemy_weights", "weights"), &EnemySpawner::set_enemy_weights);
    ClassDB::bind_method(D_METHOD("get_enemy_weights"), &EnemySpawner::get_enemy_weights);
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "enemy_weights", PROPERTY_HINT_ARRAY_TYPE, "float"), "set_enemy_weights", "get_enemy_weights");

    ClassDB::bind_method(D_METHOD("set_spawn_count", "count"), &EnemySpawner::set_spawn_count);
    ClassDB::bind_method(D_METHOD("get_spawn_count"), &EnemySpawner::get_spawn_count);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "spawn_count"), "set_spawn_count", "get_spawn_count");

    ClassDB::bind_method(D_METHOD("set_spawn_radius", "radius"), &EnemySpawner::set_spawn_radius);
    ClassDB::bind_method(D_METHOD("get_spawn_radius"), &EnemySpawner::get_spawn_radius);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spawn_radius"), "set_spawn_radius", "get_spawn_radius");

    ClassDB::bind_method(D_METHOD("set_spawn_interval", "interval"), &EnemySpawner::set_spawn_interval);
    ClassDB::bind_method(D_METHOD("get_spawn_interval"), &EnemySpawner::get_spawn_interval);
    ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "spawn_interval"), "set_spawn_interval", "get_spawn_interval");

    ClassDB::bind_method(D_METHOD("spawn_enemies"), &EnemySpawner::spawn_enemies);
    ClassDB::bind_method(D_METHOD("_on_timer_timeout"), &EnemySpawner::_on_timer_timeout);
}

EnemySpawner::EnemySpawner()
{
    spawn_count = 5;
    spawn_radius = 20.0f;
    spawn_interval = 10.0f;
}

EnemySpawner::~EnemySpawner() {}

void EnemySpawner::_ready()
{
    if (Engine::get_singleton()->is_editor_hint()) return;
    

    // シーンがエディタ側で設定されていない場合のデフォルト設定
    if (enemy_scenes.is_empty())
    {
        /*
        // ---------------------------------------------------------
        // 【ランク1】 出現しやすい（重み: 10.0）
        // ---------------------------------------------------------
        const char* rank1_paths[] = {
            "res://scenes/pinkBlob.tscn",
            "res://scenes/yeti.tscn",
            "res://scenes/chicken.tscn"
        };
        for (int i = 0; i < 3; i++) {
            Ref<PackedScene> scn = ResourceLoader::get_singleton()->load(rank1_paths[i]);
            if (scn.is_valid()) {
                enemy_scenes.append(scn);
                enemy_weights.append(10.0f); // よく出る
            }
        }*/

        // ---------------------------------------------------------
        // 【ランク2】 少し出にくい（重み: 3.0）
        // ※必要であれば、ここに中堅モンスターを追加してください
        // ---------------------------------------------------------
        const char* rank2_paths[] = {
            "res://scenes/GreenSpikyBlob.tscn",
            "res://scenes/Mushnub.tscn",
            "res://scenes/Fish.tscn"
        };
        for (int i = 0; i < 3; i++) {
            Ref<PackedScene> scn = ResourceLoader::get_singleton()->load(rank2_paths[i]);
            if (scn.is_valid()) {
                enemy_scenes.append(scn);
                enemy_weights.append(3.0f); // よく出る
            }
        }

        // ---------------------------------------------------------
        // 【ランク3】 ボス級・レア（重み: 0.5）
        // ---------------------------------------------------------
        const char* rank3_paths[] = {
            "res://scenes/Alpaking.tscn",
            "res://scenes/Armabee_Evolved.tscn",
            "res://scenes/Big_Ninja.tscn"
        };
        for (int i = 0; i < 3; i++) {
            Ref<PackedScene> scn = ResourceLoader::get_singleton()->load(rank3_paths[i]);
            if (scn.is_valid()) {
                enemy_scenes.append(scn);
                enemy_weights.append(0.5f); // 滅多に出ない
            }
        }
    }

    MultiplayerSpawner* mp_spawner = memnew(MultiplayerSpawner);
    mp_spawner->set_name("EnemyMultiplayerSpawner");
    add_child(mp_spawner);
    mp_spawner->set_spawn_path(".."); // 親ノード（EnemySpawner）を監視する

    // スポーン可能な全モンスターをSpawnerに登録
    for (int i = 0; i < enemy_scenes.size(); i++) {
        Ref<PackedScene> scn = enemy_scenes[i];
        if (scn.is_valid()) {
            mp_spawner->add_spawnable_scene(scn->get_path());
        }
    }
    if (!get_tree()->get_multiplayer()->is_server()) return;
    // 初回スポーン
    spawn_enemies();

    // タイマー設定
    spawn_timer = memnew(Timer);
    add_child(spawn_timer);
    spawn_timer->set_wait_time(spawn_interval);
    spawn_timer->set_one_shot(false);
    spawn_timer->connect("timeout", Callable(this, "_on_timer_timeout"));
    spawn_timer->start();
}

void EnemySpawner::set_enemy_scenes(const Array &scenes) { enemy_scenes = scenes; }
Array EnemySpawner::get_enemy_scenes() const { return enemy_scenes; }

void EnemySpawner::set_enemy_weights(const Array &weights) { enemy_weights = weights; }
Array EnemySpawner::get_enemy_weights() const { return enemy_weights; }

void EnemySpawner::set_spawn_count(int count) { spawn_count = count; }
int EnemySpawner::get_spawn_count() const { return spawn_count; }

void EnemySpawner::set_spawn_radius(float radius) { spawn_radius = radius; }
float EnemySpawner::get_spawn_radius() const { return spawn_radius; }

void EnemySpawner::set_spawn_interval(float interval) { spawn_interval = interval; }
float EnemySpawner::get_spawn_interval() const { return spawn_interval; }

void EnemySpawner::_on_timer_timeout()
{
    spawn_enemies();
}

void EnemySpawner::spawn_enemies()
{
    if (enemy_scenes.is_empty()) return;

    for (int i = 0; i < spawn_count; i++)
    {
        Ref<PackedScene> scene_to_spawn;

        if (enemy_weights.size() == enemy_scenes.size() && enemy_weights.size() > 0)
        {
            // 重みの合計を計算
            float total_weight = 0.0f;
            for (int j = 0; j < enemy_weights.size(); j++) {
                total_weight += (float)enemy_weights[j];
            }
            
            // ★修正: UtilityFunctions::randf() を使う (0.0 〜 1.0の乱数)
            float random_val = UtilityFunctions::randf() * total_weight;
            float current_weight = 0.0f;
            
            for (int j = 0; j < enemy_scenes.size(); j++) {
                current_weight += (float)enemy_weights[j];
                if (random_val <= current_weight) {
                    scene_to_spawn = enemy_scenes[j];
                    break;
                }
            }
        }
        else
        {
            // ウェイトがない場合は均等にランダム
            int random_index = UtilityFunctions::randi() % enemy_scenes.size();
            scene_to_spawn = enemy_scenes[random_index];
        }

        if (scene_to_spawn.is_valid())
        {
            Node3D *enemy = Object::cast_to<Node3D>(scene_to_spawn->instantiate());
            if (enemy)
            {
                // ★修正: add_childする「前」に座標を計算してセットする
                float angle = UtilityFunctions::randf() * 3.14159265f * 2.0f;
                float r = spawn_radius * std::sqrt(UtilityFunctions::randf());
                
                Vector3 spawn_pos = get_global_position();
                spawn_pos.x += r * std::cos(angle);
                spawn_pos.z += r * std::sin(angle);
                
                enemy->set_global_position(spawn_pos);

                // ★修正: force_readable_name を true にして追加（マルチプレイ同期を安定させるため）
                add_child(enemy, true);
            }
        }
    }
}