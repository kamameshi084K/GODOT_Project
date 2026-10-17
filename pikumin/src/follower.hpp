#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace godot
{
    class Follower : public CharacterBody3D
    {
        GDCLASS(Follower, CharacterBody3D);

    public:
        enum class State
        {
            Idle,       // 追従していない状態
            Following,  // ターゲットを追従している状態
            Working,    // 作業中
            Thrown,     // 投げられた状態
        };

    private:
        double speed; // 追従速度
        double gravity; // 重力
        Node3D* target_node; // 追従するターゲットノードへのポインタ
        NodePath target_path; // ターゲットノードのパス
        State current_state; // フォロワーの状態

    protected:
        // Godotのエディタと変数を連携するためのメソッド
        static void _bind_methods();

    public:
        Follower();
        ~Follower();

        // Godotの標準関数をオーバーライドa
        void _ready() override;
        void _physics_process(double delta) override;

        // Godotのエディタで設定可能なプロパティのゲッターとセッター
        double get_speed() const;
        void set_speed(double p_speed);

        NodePath get_target_path() const;
        void set_target_path(const NodePath& p_target_path);

        void set_state(State p_state);
        State get_state() const;

        void throw_to(Vector3 p_target_pos);
    };
}