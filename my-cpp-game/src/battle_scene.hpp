#pragma once

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>

namespace godot
{
    class BattleScene : public Node3D
    {
        GDCLASS(BattleScene, Node3D)

    private:
        Marker3D* player_spawn_pos;
        Marker3D* enemy_spawn_pos;

        // モンスター名から、3Dモデルのパスを取得する関数
        String _get_model_path_by_name(const String& name);

    protected:
        static void _bind_methods();

    public:
        BattleScene(); // コンストラクタ
        ~BattleScene(); // デストラクタ

        virtual void _ready() override; // シーン準備完了時の処理

        /**
         * @brief 指定されたモンスター名に基づいて敵をスポーンさせる
         * 
         * @param monster_name スポーンさせるモンスターの名前
         */
        void _rpc_spawn_enemy(const String& monster_name);
    };
}