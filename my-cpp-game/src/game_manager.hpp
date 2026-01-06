#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>

namespace godot
{

    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)

    private:
        static GameManager *singleton; // 自分自身（シングルトン）へのポインタ
        
        Vector3 last_player_position;  // プレイヤーの座標
        bool is_returning_from_battle; // バトルから帰ってきたかフラグ

    protected:
        static void _bind_methods();

    public:
        GameManager();
        ~GameManager();

        // どこからでも GameManager を呼べるようにする
        static GameManager *get_singleton();

        // データのセット・ゲット

        /**
         * @brief プレイヤーの最後の座標を設定する
         * 
         * @param pos 設定する座標
         * @note const reference を使って効率的に引数を渡す
         */
        void set_last_player_position(const Vector3 &pos);

        /**
         * @brief プレイヤーの最後の座標を取得する
         * 
         * @return Vector3 最後の座標
         */
        Vector3 get_last_player_position() const;

        /**
         * @brief バトルから帰ってきたかどうかのフラグを設定する
         * 
         * @param value 設定するフラグの値
         */
        void set_is_returning_from_battle(bool value);

        /**
         * @brief バトルから帰ってきたかどうかのフラグを取得する
         * 
         * @return true バトルから帰ってきた
         * @return false バトルから帰ってきていない
         */
        bool get_is_returning_from_battle() const;
    };

}