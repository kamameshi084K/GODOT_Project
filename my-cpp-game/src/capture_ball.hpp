#pragma once
#include <godot_cpp/classes/area3d.hpp>

namespace godot
{

    /**
     * @class CaptureBall
     * @brief モンスターを捕獲するためのボールオブジェクト
     */
    class CaptureBall : public Area3D
    {
        GDCLASS(CaptureBall, Area3D)

    private:
        // ボールの移動方向
        Vector3 direction;
        // ボールの移動速度
        float speed;
        // 生存時間（秒）
        float life_time;

    protected:
        // Godotへのメソッド登録用
        static void _bind_methods();

    public:
        // コンストラクタ
        CaptureBall();
        // デストラクタ
        ~CaptureBall();

        // ノード準備完了時の処理
        virtual void _ready() override;

        /**
         * @brief 発射時のセットアップ
         * @param dir 飛ばす方向のベクトル
         */
        void setup(Vector3 dir);

        /**
         * @brief 毎フレームの更新処理
         * @param delta 前フレームからの経過時間
         */
        virtual void _process(double delta) override;

        /**
         * @brief 他の物体と衝突した際のイベントハンドラ
         * @param body 衝突した対象のノード
         */
        void _on_body_entered(Node3D* body);
    };

}