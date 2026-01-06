#include "enemy.hpp"
#include "game_manager.hpp" // GameManagerを使うために必要
#include <godot_cpp/core/class_db.hpp> // ClassDBを使うために必要
#include <godot_cpp/classes/engine.hpp> // Engineを使うために必要
#include <godot_cpp/classes/scene_tree.hpp> // SceneTreeを使うために必要
#include <godot_cpp/variant/utility_functions.hpp> // UtilityFunctionsを使うために必要
#include <godot_cpp/classes/kinematic_collision3d.hpp> // KinematicCollision3Dを使うために必要

// 名前空間を使用
// namespaceを使うことで、godot:: を毎回書かなくて済むように
using namespace godot;

void Enemy::_bind_methods()
{
    // 必要ならここにプロパティ登録などを書きます
}

/**
 * @brief コンストラクタ（初期化）
 */
Enemy::Enemy()
{
    speed = 3.0;
    detection_range = 10.0; // 10メートル以内に入ったら気づく
    gravity = 9.8;
}

/**
 * @brief デストラクタ
 */
Enemy::~Enemy()
{
}

/**
 * @brief キャラクターの動作を制御するために_physics_processをオーバーライド
 * @param delta フレーム間の経過時間（秒）
 * @note virtualとは、基底クラスで仮想関数として宣言されていることを示します
 */
void Enemy::_physics_process(double delta)
{
    // エディタ上では動かさない
    if (Engine::get_singleton()->is_editor_hint()) return;

    // 敵の基本的な物理処理
    Vector3 velocity = get_velocity();

    // 1. 重力 (地面にいないなら落下させる)
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta;
    }

    // 2. プレイヤーを検出して追いかける
    Node* player_node = get_tree()->get_first_node_in_group("player");
    if (player_node)
    {
        Node3D* player_3d = Object::cast_to<Node3D>(player_node);
        if (player_3d)
        {
            Vector3 my_pos = get_global_position();
            Vector3 player_pos = player_3d->get_global_position();
            
            // 距離チェック
            double dist = my_pos.distance_to(player_pos);
            if (dist < detection_range)
            {
                // ターゲット(敵) - 自分 = 進むべき方向
                Vector3 direction = (player_pos - my_pos).normalized();
                direction.y = 0; // 空には飛ばない

                // 移動
                velocity.x = direction.x * speed;
                velocity.z = direction.z * speed;

                // プレイヤーの方を向く
                if (direction.length_squared() > 0.01)
                {
                    // ※ look_atのターゲットは「座標」なので player_pos を使う
                    // そのままだと足元を見て前のめりになるので、高さを自分に合わせる
                    Vector3 look_target = player_pos;
                    look_target.y = my_pos.y;
                    look_at(look_target, Vector3(0, 1, 0));
                }
            }
            else
            {
                // プレイヤーが範囲外なら停止
                velocity.x = Math::move_toward(velocity.x, (real_t)0.0, (real_t)(10.0 * delta));
                velocity.z = Math::move_toward(velocity.z, (real_t)0.0, (real_t)(10.0 * delta));
            }
        }
    }
    set_velocity(velocity);
    move_and_slide();

    // 移動した結果、何かにぶつかっているかチェック
    for (int i = 0; i < get_slide_collision_count(); i++)
    {
        Ref<KinematicCollision3D> collision = get_slide_collision(i);
        Object *collider = collision->get_collider();

        // ぶつかった相手をNodeとして扱い、グループを確認
        Node *body = Object::cast_to<Node>(collider);

        // "player" グループに入っている物体（プレイヤー）ならバトルへ
        if (body && body->is_in_group("player"))
        {
            GameManager *gm = GameManager::get_singleton(); // クラス名は GameManager でOK
            if (gm)
            {
                Node3D *player_3d = Object::cast_to<Node3D>(body);
                if (player_3d)
                {
                    // プレイヤーの現在位置を保存
                    gm->set_last_player_position(player_3d->get_global_position());
                    // 「バトル帰りだよ」フラグを立てる
                    gm->set_is_returning_from_battle(true);
                    
                    UtilityFunctions::print("Position Saved: ", player_3d->get_global_position());
                }
            }
            UtilityFunctions::print("Encounter! Battle Start!");

            // 物理演算中にシーンを変えるため、call_deferredを使用
            get_tree()->call_deferred("change_scene_to_file", "res://battle.tscn");
            
            // バトル開始後はこれ以上処理しない
            return;
        }
    }
}