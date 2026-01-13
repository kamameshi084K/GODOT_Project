#pragma

#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/variant/typed_array.hpp>

namespace godot
{
    class EnemySpawner : public Node3D
    {
        GDCLASS(EnemySpawner, Node3D)

    private:
        // エディタで設定する敵のリスト（例: [Bird.tscn, Ninja.tscn]）
        TypedArray<PackedScene> enemy_scenes;
        
        // 生成した敵への参照（死んだかどうかの確認用）
        Node* current_enemy;

        // 次に湧くまでのタイマー
        double respawn_timer;
        
        // 設定値
        double spawn_radius; // プレイヤーが何メートル近づいたら湧くか
        double respawn_delay; // 倒されてから次が湧くまでの時間

    protected:
        static void _bind_methods();

    public:
        EnemySpawner();
        ~EnemySpawner();

        virtual void _process(double delta) override;

        /**
         * @brief ランダムな敵をスポーンさせる
         * 
         */
        void spawn_random_enemy();

        // セッター・ゲッター
        /**
         * @brief 敵シーンのリストを設定する
         * @param p_scenes 敵シーンの配列
         */
        void set_enemy_scenes(const TypedArray<PackedScene>& p_scenes);
        /**
         * @brief 敵シーンのリストを取得する
         * @return TypedArray<PackedScene> 敵シーンの配列
         */
        TypedArray<PackedScene> get_enemy_scenes() const;
    };
} // namespace godot