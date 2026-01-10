#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/vector3.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/packed_string_array.hpp>

namespace godot
{

    class GameManager : public Node
    {
        GDCLASS(GameManager, Node)

    private:
        static GameManager *singleton; // 自分自身（シングルトン）へのポインタ
        
        Vector3 last_player_position;  // プレイヤーの座標
        bool is_returning_from_battle; // バトルから帰ってきたかフラグ
        String next_enemy_scene_path;  // 次に戦う敵のシーンパス
        String current_enemy_id; // 今戦っている敵のID
        PackedStringArray defeated_enemies; // 倒された敵のIDリスト
        String next_enemy_name; // 次に戦う敵の名前
        int next_enemy_max_hp; // 次に戦う敵の最大HP
        int next_enemy_attack; // 次に戦う敵の攻撃力
        int player_attack;  // プレイヤー攻撃力
        int player_defense; // プレイヤー防御力
        int next_enemy_defense; // 次に戦う敵の防御力

    protected:
        /**
         * @brief Godot にメソッドを登録する
         * @return void
         * @note _bind_methods は protected メソッドとして宣言されている必要があります
         */
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

        /**
         * @brief 次に戦う敵のシーンパスを設定する
         * 
         * @param path シーンパス
         */
        void set_next_enemy_scene_path(const String &path);

        /**
         * @brief 次に戦う敵のシーンパスを取得する
         * 
         * @return String シーンパス
         */
        String get_next_enemy_scene_path() const;

        /**
         * @brief 現在戦っている敵のIDを設定する
         * 
         * @param id 敵のID
         */
        void set_current_enemy_id(const String &id);
        /**
         * @brief 現在戦っている敵のIDを取得する
         * 
         * @return String 敵のID
         */
        String get_current_enemy_id() const;

        /**
         * @brief 倒した敵のIDをリストに追加する
         * 
         * @param id 
         * @note 同じIDが複数回追加されることはないようにする
         */
        void add_defeated_enemy(const String &id);
        /**
         * @brief 敵が倒されたかどうかを確認する
         * 
         * @param id 敵のID
         * @return - true: 敵は倒された
         * @return - false: 敵はまだ生きている
         */
        bool is_enemy_defeated(const String &id) const;

        /**
         * @brief 次に戦う敵のステータスを設定する
         * 
         * @param name 敵の名前
         * @param hp 敵の最大HP
         * @param attack 敵の攻撃力
         */
        void set_next_enemy_stats(const String &name, int hp, int attack);
        
        /**
         * @brief 次に戦う敵の名前を取得する
         * 
         * @return String 敵の名前
         */
        String get_next_enemy_name() const;
        /**
         * @brief 次に戦う敵の最大HPを取得する
         * 
         * @return int 敵の最大HP
         */
        int get_next_enemy_max_hp() const;
        /**
         * @brief 次に戦う敵の攻撃力を取得する
         * 
         * @return int 敵の攻撃力
         */
        int get_next_enemy_attack() const;

        /**
         * @brief プレイヤーのステータスを設定する
         * 
         * @param attack 攻撃力
         * @param defense 防御力
         */
        void set_player_stats(int attack, int defense); 
        /**
         * @brief プレイヤーの攻撃力を取得する
         * 
         * @return int 攻撃力
         */
        int get_player_attack() const;
        /**
         * @brief プレイヤーの防御力を取得する
         * 
         * @return int 防御力
         */
        int get_player_defense() const;

        /**
         * @brief 次に戦う敵の防御力を設定する
         * 
         * @param def 敵の防御力
         */
        void set_next_enemy_defense(int def);
        /**
         * @brief 次に戦う敵の防御力を取得する
         * 
         * @return int 敵の防御力
         */
        int get_next_enemy_defense() const;
    };
}