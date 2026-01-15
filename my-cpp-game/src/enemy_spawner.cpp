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
    // 1. すでに敵がいる場合：デスポーン判定
    if (get_child_count() > 0)
    {
        Viewport* viewport = get_viewport();
        if (!viewport) return;
        Camera3D* camera = viewport->get_camera_3d();
        if (!camera) return;

        Node3D* enemy = Object::cast_to<Node3D>(get_child(0));
        if (enemy)
        {
            // 距離を計算（カメラと敵の距離）
            double dist = camera->get_global_position().distance_to(enemy->get_global_position());
            
            // 画面に映っているか？
            bool is_visible = camera->is_position_in_frustum(enemy->get_global_position());

            // --- デスポーン条件の設定 ---
            
            // 条件A: 「画面外」かつ「20m以上離れている」
            bool condition_soft = (!is_visible && dist > 20.0);
            
            // 条件B: 「（見ていても）50m以上離れている」
            // ※ここを20.0にすると、目の前で消えてしまいます！お好みで調整してください。
            bool condition_hard = (dist >= 30.0);

            // どちらか（OR）を満たしたら消去
            if (condition_soft || condition_hard)
            {
                enemy->queue_free(); // 消去
                
                respawn_timer = 0.0; // すぐに次が湧ける状態にする（必要なら数秒待つ設定へ）
                UtilityFunctions::print("Enemy despawned. Dist: ", dist, " Visible: ", is_visible);
            }
        }
        return; 
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

    Viewport* viewport = get_viewport();
    if (!viewport) return;
    Camera3D* camera = viewport->get_camera_3d();
    if (!camera) return;

    // 自分（スポナー）とカメラの距離を測る
    double dist_to_spawner = camera->get_global_position().distance_to(get_global_position());

    // 湧く条件：
    // 1. カメラに映っている
    // 2. 「消える距離(50m)」よりも内側に入り込んでいる（例: 40m以内）
    if (camera->is_position_in_frustum(get_global_position()) && dist_to_spawner < 30.0)
    {
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