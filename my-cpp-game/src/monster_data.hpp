#pragma once
#include "skill_data.hpp"

#include <godot_cpp/classes/resource.hpp> // RefCounted ではなく Resource を使う
#include <godot_cpp/variant/typed_array.hpp> // 配列を使うために必要

namespace godot
{
    // モンスターの特性
    enum MonsterAbility
    {
        ABILITY_NONE = 0,
        ABILITY_PHYSICAL_BOOST = 1, // 物理技の威力 1.5倍
        ABILITY_SPECIAL_BOOST = 2   // 特殊技の威力 1.5倍
    };

    // モンスターのデータクラス
    // resourceとして扱うことで、.tres や .res ファイルに保存できるようにする
    class MonsterData : public Resource
    {
        GDCLASS(MonsterData, Resource)

    private:
        String monster_name; // モンスターの名前
        String id;           // モンスターの一意なID
        int max_hp;          // 最大HP
        int attack;          // 攻撃力
        int defense;         // 防御力
        int speed;           // 素早さ

        // 特性
        int ability;

        // 技リスト（SkillDataのリソース配列）
        TypedArray<SkillData> skills;

        // 3Dモデルのパス（シーンの読み込み用）
        String model_path;

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

        /**
         * @brief 特性の設定・取得
         * @param val MonsterAbility のいずれか
         */
        void set_ability(int val);
        /**
         * @brief 特性の取得
         * @return int 特性
         */
        int get_ability() const;

        /**
         * @brief 技リストの設定・取得
         * @param p_skills SkillData の配列
         */
        void set_skills(const TypedArray<SkillData>& p_skills);
        /**
         * @brief 技リストの取得
         * @return TypedArray<SkillData> 技リスト
         */
        TypedArray<SkillData> get_skills() const;

        /**
         * @brief 3Dモデルのパス設定・取得
         * @param path シーンファイルのパス
         */
        void set_model_path(const String& path);
        /**
         * @brief 3Dモデルのパス取得
         * @return String シーンファイルのパス
         */
        String get_model_path() const;
    };
}