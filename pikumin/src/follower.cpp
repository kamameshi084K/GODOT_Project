#include "follower.hpp"
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/project_settings.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

// Godotエディタの「インスペクター」に変数（speedやtarget_path）を表示させる設定
void Follower::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_speed", "speed"), &Follower::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &Follower::get_speed);
    ClassDB::add_property("Follower", PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_target_path", "path"), &Follower::set_target_path);
    ClassDB::bind_method(D_METHOD("get_target_path"), &Follower::get_target_path);
    ClassDB::add_property("Follower", PropertyInfo(Variant::NODE_PATH, "target_path"), "set_target_path", "get_target_path");
}

Follower::Follower()
{
    speed = 5.0; // デフォルトの追従速度を設定
    target_node = nullptr; // ターゲットノードは初期化時にはnullptr
    gravity = ProjectSettings::get_singleton()->get_setting("physics/3d/default_gravity"); // プロジェクト設定から重力を取得
    current_state = State::Idle; // 初期状態をIdleに設定
}

Follower::~Follower()
{
    // デストラクタで特別な処理は必要ありません
}

void Follower::_ready()
{
// エディタ画面でのプレビュー中は動かないようにする
    if (Engine::get_singleton()->is_editor_hint()) return;

    add_to_group("followers"); // このノードを "followers" グループに追加

    // 指定されたパスからターゲット（プレイヤー）の本体を取得する
    if (!target_path.is_empty())
    {
        target_node = Object::cast_to<Node3D>(get_node_or_null(target_path));
    }
}

void Follower::_physics_process(double delta)
{
    // エディタ画面でのプレビュー中は動かないようにする
    if (Engine::get_singleton()->is_editor_hint()) return;

    // 現在の速度を取得
    Vector3 velocity = get_velocity();

    if (!is_on_floor())
    {
        // 空中にいる場合は重力を適用
        velocity.y -= gravity * delta;
    }

    switch (current_state)
    {
    case State::Idle:
        // Idle状態での処理
        velocity.x = 0;
        velocity.z = 0;
        break;
    case State::Following:
        // 追従状態での処理
        // ターゲットノードが存在する場合のみ追従処理を行う
        if (target_node)
        {
            // 現在の位置とターゲットの位置を取得
            Vector3 current_position = get_global_transform().origin;
            Vector3 target_position = target_node->get_global_transform().origin;

            target_position.y = current_position.y; // Y軸の高さは固定して追従する

            // ターゲットとの距離を計算
            float distance = current_position.distance_to(target_position);

            Vector3 move_dir = Vector3(0, 0, 0);

            if (distance > 2.0f)
            {
                move_dir = current_position.direction_to(target_position);
            }

            Vector3 separation_dir = Vector3(0, 0, 0);

            TypedArray<Node> peers = get_tree()->get_nodes_in_group("followers");

            for (int i = 0; i < peers.size(); i++)
            {
                Node3D* peer = Object::cast_to<Node3D>(peers[i]);

                if (peer != this)
                {
                    float dist_to_peer = current_position.distance_to(peer->get_global_position());
                    
                    // もし1.0m以内に仲間がいたら反発する
                    if (dist_to_peer < 1.0f && dist_to_peer > 0.0f)
                    {
                        Vector3 push_away = peer->get_global_position().direction_to(current_position);
                        // 近いほど強く反発するように重み付け
                        separation_dir += push_away * (1.0f - dist_to_peer);
                    }
                }
            }

            Vector3 final_direction = (move_dir + separation_dir * 2.0f).normalized();

            if (distance > 2.0f || separation_dir.length() > 0.1f)
            {
                velocity.x = final_direction.x * speed;
                velocity.z = final_direction.z * speed;
            }
            else
            {
                velocity.x = 0;
                velocity.z = 0;
            }
        }
        break;

    case State::Working:
        // 作業中状態での処理
        break;
    case State::Thrown:
        // 【修正】落下中（Yの速度が0以下）の時だけ、床に着地したかを判定する
        if (velocity.y <= 0.0f && is_on_floor())
        {
            // 着地したら移動をピタッと止めて、待機状態（Idle）に戻る
            velocity.x = 0;
            velocity.z = 0;
            set_state(State::Idle);
        }
        break;

    default:
        break;
    }

    set_velocity(velocity);
    move_and_slide();
}

void Follower::throw_to(Vector3 p_target_pos)
{
    // 状態を「投げられ中」にする
    set_state(Follower::State::Thrown);

    // 自分の現在地を取得（Player側ですでに頭上にワープさせられた後の座標になります）
    Vector3 current_pos = get_global_position();
    
    // ターゲットまでの差分（距離と高さ）を計算
    Vector3 diff = p_target_pos - current_pos;

    // 滞空時間（将来的に個体の色や重さごとに変えたい場合は、ここを調整します）
    float fly_time = 1.0f; 

    // 物理演算の逆算で初速ベクトルを自ら計算する
    Vector3 throw_velocity = Vector3(0, 0, 0);
    throw_velocity.x = diff.x / fly_time;
    throw_velocity.z = diff.z / fly_time;
    throw_velocity.y = (diff.y + 0.5f * gravity * fly_time * fly_time) / fly_time;

    // 計算した初速をセットして飛ぶ
    set_velocity(throw_velocity);
}

void Follower::set_speed(double p_speed) { speed = p_speed; }
double Follower::get_speed() const { return speed; }
void Follower::set_target_path(const NodePath& p_target_path) { target_path = p_target_path; }
NodePath Follower::get_target_path() const { return target_path; }
void Follower::set_state(State p_state) { current_state = p_state; }
Follower::State Follower::get_state() const { return current_state; }
