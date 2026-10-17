#include "player.hpp"
#include "follower.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/viewport.hpp>
#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/world3d.hpp>
#include <godot_cpp/classes/physics_direct_space_state3d.hpp>
#include <godot_cpp/classes/physics_ray_query_parameters3d.hpp>
#include <godot_cpp/variant/dictionary.hpp>

using namespace godot;

void Player::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_aim_cursor_path", "path"), &Player::set_aim_cursor_path);
    ClassDB::bind_method(D_METHOD("get_aim_cursor_path"), &Player::get_aim_cursor_path);
    ClassDB::add_property("Player", PropertyInfo(Variant::NODE_PATH, "aim_cursor_path"), "set_aim_cursor_path", "get_aim_cursor_path");
}

Player::Player()
{
    speed = 5.0; // デフォルトの移動速度を設定
    gravity = ProjectSettings::get_singleton()->get_setting("physics/3d/default_gravity"); // プロジェクト設定から重力を取得
}

Player::~Player()
{
    // デストラクタで特別な処理は必要ありません
}

void Player::_ready()
{
    // 初期化処理が必要な場合はここに記述
    if (!aim_cursor_path.is_empty())
    {
        aim_cursor_node = Object::cast_to<Node3D>(get_node_or_null(aim_cursor_path));
    }
}

void Player::_physics_process(double delta)
{
    if (Engine::get_singleton()->is_editor_hint()) return; // エディタ画面でのプレビュー中は動かないようにする

    Vector3 velocity = get_velocity(); // 現在の速度を取得

    // 重力の処理
    if (!is_on_floor())
    {
        velocity.y -= gravity * delta; // 空中にいる場合は重力を適用
    }

    Input *input = Input::get_singleton(); // 入力の取得
    Vector2 input_dir = input->get_vector("move_left", "move_right", "move_up", "move_down"); // 入力方向の取得

    Vector3 direction = Vector3(input_dir.x, 0, input_dir.y); // 入力方向を3Dベクトルに変換

    if (direction.length() > 0)
    {
        direction = direction.normalized(); // 入力方向を正規化
        velocity.x = direction.x * speed; // X方向の速度を設定
        velocity.z = direction.z * speed; // Z方向の速度を設定
    }
    else
    {
        velocity.x = 0; // 入力がない場合はX方向の速度を0にする
        velocity.z = 0; // 入力がない場合はZ方向の速度を0にする
    }

    if (input->is_action_just_pressed("whistle"))
    {
        // "followers" グループにいる全キャラクターを取得
        TypedArray<Node> followers = get_tree()->get_nodes_in_group("followers");
        for (int i = 0; i < followers.size(); i++)
        {
            // Node型からFollower型にキャスト
            Follower *follower = Object::cast_to<Follower>(followers[i]);

            if (follower != nullptr)
            {
                if (follower->get_state() == Follower::State::Idle)
                {
                    // フォロワーがIdle状態の場合、Following状態に変更
                    follower->set_state(Follower::State::Following);
                }
            }
        }
    }

    Vector2 aim_input = input->get_vector("aim_left", "aim_right", "aim_up", "aim_down");
    
    if (aim_cursor_node != nullptr)
    {
        if (aim_input.length() > 0.1f)
        {
            // ① まずスティックの入力から、目指すべき X と Z の座標（水平位置）を計算する
            Vector3 aim_dir = Vector3(aim_input.x, 0, aim_input.y);
            Vector3 base_target = get_global_position() + aim_dir * 8.0f; // 8.0f は投げる最大距離

            // ② レイキャスト（真上から真下へ光線を撃つ）の準備
            // プレイヤーの現在地より少し高い位置から、深い地下に向かって光線を設定
            Vector3 ray_start = Vector3(base_target.x, get_global_position().y + 10.0f, base_target.z);
            Vector3 ray_end   = Vector3(base_target.x, get_global_position().y - 10.0f, base_target.z);

            PhysicsDirectSpaceState3D* space_state = get_world_3d()->get_direct_space_state();
            Ref<PhysicsRayQueryParameters3D> query = PhysicsRayQueryParameters3D::create(ray_start, ray_end);
            
            // ③ 地面（レイヤー1）だけに当たるようにマスクを設定（AIやプレイヤーを貫通させる）
            query->set_collision_mask(1);

            // ④ 光線を撃って結果を取得！
            Dictionary result = space_state->intersect_ray(query);

            if (!result.is_empty())
            {
                // 地面にぶつかった場合、その衝突座標（hit_pos）をカーソルの位置にする
                Vector3 hit_pos = result["position"];
                
                // 地面と全く同じ高さだとチラつく（Zファイティング）ため、0.02mだけ浮かす
                aim_cursor_node->set_global_position(hit_pos + Vector3(0, 0.02f, 0));
            }
            else
            {
                // 万が一地面がなかった場合（崖の外などを狙った場合）の安全対策
                aim_cursor_node->set_global_position(Vector3(base_target.x, get_global_position().y, base_target.z));
            }

            aim_cursor_node->set_visible(true);
        }
        else
        {
            aim_cursor_node->set_visible(false);
        }
    }

    // カーソルが表示されている（右スティックを倒している）状態でのみ投げられるようにする
    if (input->is_action_just_pressed("throw") && aim_cursor_node != nullptr && aim_cursor_node->is_visible()) {
        
        Vector3 target_pos = aim_cursor_node->get_global_position();

        // 追従中のAIの中で、一番プレイヤーに近い個体を探す
        TypedArray<Node> followers = get_tree()->get_nodes_in_group("followers");
        Follower* closest_follower = nullptr;
        float min_dist = 10000.0f;

        for (int i = 0; i < followers.size(); i++) {
            Follower* f = Object::cast_to<Follower>(followers[i]);
            if (f != nullptr && f->get_state() == Follower::State::Following) {
                float dist = get_global_position().distance_to(f->get_global_position());
                if (dist < min_dist) {
                    min_dist = dist;
                    closest_follower = f;
                }
            }
        }

        if (closest_follower != nullptr)
        {
            // プレイヤーの当たり判定に引っかかって真上に飛ぶのを防ぐため、
            // プレイヤーの頭上（Y方向に +2.0m）に移動させる
            Vector3 spawn_pos = get_global_position() + Vector3(0, 1.0f, 0); 
            closest_follower->set_global_position(spawn_pos);

            // Followerに「カーソルの座標（target_pos）へ飛んでいけ！」と命令するだけ
            closest_follower->throw_to(target_pos);
        }
    }

    set_velocity(velocity); // 計算した速度を設定
    move_and_slide(); // キャラクターを移動させる
}

void Player::set_aim_cursor_path(const NodePath& p_path) { aim_cursor_path = p_path; }
NodePath Player::get_aim_cursor_path() const { return aim_cursor_path; }