#pragma once

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/animation_player.hpp> // cppに合わせてAnimationPlayerに戻しました
#include <godot_cpp/classes/animation_tree.hpp>   // 追加: AnimationTreeを使うためにインクルード
#include <godot_cpp/classes/animation_node_state_machine_playback.hpp>
#include <godot_cpp/classes/animation_node_one_shot.hpp>
#include <godot_cpp/classes/area3d.hpp>

namespace godot
{
    class Player : public CharacterBody3D
    {
        GDCLASS(Player, CharacterBody3D)

    private:
        double speed;
        double jump_velocity;
        double gravity;
        double camera_sensitivity;
        double acceleration;
        double friction;

        // cpp側で anim_player を使っているため、型と変数名を合わせました
        AnimationPlayer* anim_player;
        // ▼ 変更: AnimationTree* に変える
        AnimationTree* anim_tree;
        AnimationNodeStateMachinePlayback* state_machine; // 追加: ステートマシンの操作用

    protected:
        static void _bind_methods();

    public:
        Player();
        ~Player();
        virtual void _ready() override;
        virtual void _physics_process(double delta) override;
    };
}