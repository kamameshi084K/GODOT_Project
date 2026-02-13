#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/timer.hpp>

namespace godot
{
    class EnemySpawner : public Node3D
    {
        GDCLASS(EnemySpawner, Node3D)

    private:
        Array enemy_scenes;
        Array enemy_weights; // ★追加: 各シーンの出現確率（重み）
        
        int spawn_count;
        float spawn_radius;
        float spawn_interval;
        
        Timer* spawn_timer;

    protected:
        static void _bind_methods();

    public:
        EnemySpawner();
        ~EnemySpawner();

        virtual void _ready() override;

        void set_enemy_scenes(const Array &scenes);
        Array get_enemy_scenes() const;

        // ★追加: ウェイト用のゲッター/セッター
        void set_enemy_weights(const Array &weights);
        Array get_enemy_weights() const;

        void set_spawn_count(int count);
        int get_spawn_count() const;

        void set_spawn_radius(float radius);
        float get_spawn_radius() const;

        void set_spawn_interval(float interval);
        float get_spawn_interval() const;

        void spawn_enemies();
        void _on_timer_timeout();
    };
}