#pragma once

#include <godot_cpp/classes/ref_counted.hpp>
#include <godot_cpp/variant/string.hpp>

namespace godot
{
    // RefCountedを継承することで、Ref<MonsterData> としてスマートポインタで管理できるようになる
    // RefCountedを継承するとは、Godotの参照カウント型オブジェクトとして扱うことを意味します。
    // これにより、Ref<MonsterData> のようなスマートポインタでMonsterDataオブジェクトを管理でき、
    // メモリ管理が自動化され、メモリリークのリスクが軽減されます。
    class MonsterData : public RefCounted
    {
        GDCLASS(MonsterData, RefCounted)

    private:
        String monster_name; // モンスターの名前
        String id;           // モンスターの一意なID
        int max_hp;          // 最大HP
        int attack;          // 攻撃力
        int defense;         // 防御力
        int speed;           // 素早さ

    protected:
        /**
         * @brief Godotのクラスバインディングメソッド
         * 
         */
        static void _bind_methods();

    public:
        MonsterData();
        ~MonsterData();

        // ステータスを一括設定する便利関数
        /**
         * @brief ステータス一括設定
         * 
         * @param p_hp 最大HP
         * @param p_atk 攻撃力
         * @param p_def 防御力
         * @param p_spd 素早さ
         */
        void set_stats(int p_hp, int p_atk, int p_def, int p_spd);

        // セッター・ゲッター
        /**
         * @brief モンスター名の設定・取得
         * @param name モンスター名
         */
        void set_monster_name(const String& name);
        /**
         * @brief モンスター名の取得
         * @return String モンスター名
         */
        String get_monster_name() const;

        /**
         * @brief モンスターIDの設定・取得
         * @param p_id モンスターID
         */
        void set_id(const String& p_id);
        /**
         * @brief モンスターIDの取得
         * @return String モンスターID
         */
        String get_id() const;

        /**
         * @brief 各ステータスのセッター・ゲッター
         * @param val ステータス値
         */
        void set_max_hp(int val);
        /**
         * @brief 最大HPの取得
         * @return int 最大HP
         */
        int get_max_hp() const;

        /**
         * @brief 攻撃力の設定・取得
         * @param val 攻撃力
         */
        void set_attack(int val);
        /**
         * @brief 攻撃力の取得
         * @return int 攻撃力
         */
        int get_attack() const;

        /**
         * @brief 防御力の設定・取得
         * @param val 防御力
         */
        void set_defense(int val);
        /**
         * @brief 防御力の取得
         * @return int 防御力
         */
        int get_defense() const;

        /**
         * @brief 素早さの設定・取得
         * @param val 素早さ
         */
        void set_speed(int val);
        /**
         * @brief 素早さの取得
         * @return int 素早さ
         */
        int get_speed() const;
    };
}