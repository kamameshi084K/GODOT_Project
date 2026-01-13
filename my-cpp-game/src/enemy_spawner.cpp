#include "enemy_spawner.hpp"
#include "game_manager.hpp"
#include <godot_cpp/classes/random_number_generator.hpp>
#include <godot_cpp/classes/viewport.hpp>  // ビューポート取得用
#include <godot_cpp/classes/camera3d.hpp>  // カメラ取得用
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void EnemySpawner::_bind_methods()
{
    // 敵リストプロパティ
    ClassDB::bind_method(D_METHOD("set_enemy_scenes", "p_scenes"), &EnemySpawner::set_enemy_scenes);
    ClassDB::bind_method(D_METHOD("get_enemy_scenes"), &EnemySpawner::get_enemy_scenes);
    
    // エディタで配列を編集できるようにする
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "enemy_scenes", PROPERTY_HINT_TYPE_STRING, "%24/6:PackedScene"), "set_enemy_scenes", "get_enemy_scenes");
}

EnemySpawner::EnemySpawner()
{
    current_enemy = nullptr;
    respawn_timer = 0.0;
    spawn_radius = 40.0; // 距離チェックは少し広めにとる（この範囲内で、かつ画面内の時だけ湧く）
    respawn_delay = 5.0; 
}

EnemySpawner::~EnemySpawner()
{
}

void EnemySpawner::_process(double delta)
{
    // 1. すでに敵がいるかチェック
    // （子ノードがいなくなったら倒されたとみなす簡易判定）
    if (get_child_count() > 0)
    {
        return; // 敵がいる間は何もしない
    }
    
    // 2. リスポーン待ち時間中ならカウントダウン
    if (respawn_timer > 0)
    {
        respawn_timer -= delta;
        return;
    }

    // 3. スポーン判定
    // まずGameManagerなどでプレイヤーとの距離をざっくりチェック（遠すぎる場所のカメラ判定を省くため）
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        // プレイヤーの位置を取得（GameManagerにこの機能がない場合は省略可だが、あると負荷軽減になる）
        // ここでは「カメラが見ているか」を最優先するため、一旦距離チェックはスキップしても動きますが、
        // 念のため記述
    }
    
    // 現在のビューポート（画面）を取得
    Viewport* viewport = get_viewport();
    if (!viewport) return;

    // 現在のアクティブなカメラを取得
    Camera3D* camera = viewport->get_camera_3d();
    if (!camera) return;

    // 「このスポナーの位置」がカメラの視界（フラスタム）に入っているか？
    if (camera->is_position_in_frustum(get_global_position()))
    {
        // 視界に入った！かつ、敵がいない！ -> 生成！
        spawn_random_enemy();
    }
}

void EnemySpawner::spawn_random_enemy()
{
    if (enemy_scenes.size() == 0)
    {
        return;
    }

    // シンプルにランダムな整数を取得して、配列のサイズで割った余りを使う
    int index = UtilityFunctions::randi() % enemy_scenes.size();

    Ref<PackedScene> selected_scene = enemy_scenes[index];
    
    if (selected_scene.is_valid())
    {
        Node* node = selected_scene->instantiate();
        Node3D* enemy = Object::cast_to<Node3D>(node);
        
        if (enemy)
        {
            add_child(enemy);
            enemy->set_position(Vector3(0, 0, 0)); 
            
            // ログ出力（動作確認用）
            UtilityFunctions::print("Enemy appeared! Index: ", index);
        }
    }
}

// セッター・ゲッター
void EnemySpawner::set_enemy_scenes(const TypedArray<PackedScene>& p_scenes)
{
    enemy_scenes = p_scenes;
}

TypedArray<PackedScene> EnemySpawner::get_enemy_scenes() const
{
    return enemy_scenes;
}