#include "monster_data.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MonsterData::_bind_methods()
{
    // --- 既存のプロパティ ---
    ClassDB::bind_method(D_METHOD("set_monster_name", "name"), &MonsterData::set_monster_name);
    ClassDB::bind_method(D_METHOD("get_monster_name"), &MonsterData::get_monster_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "monster_name"), "set_monster_name", "get_monster_name");

    ClassDB::bind_method(D_METHOD("set_id", "id"), &MonsterData::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &MonsterData::get_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "set_id", "get_id");

    ClassDB::bind_method(D_METHOD("set_max_hp", "val"), &MonsterData::set_max_hp);
    ClassDB::bind_method(D_METHOD("get_max_hp"), &MonsterData::get_max_hp);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_hp"), "set_max_hp", "get_max_hp");

    ClassDB::bind_method(D_METHOD("set_attack", "val"), &MonsterData::set_attack);
    ClassDB::bind_method(D_METHOD("get_attack"), &MonsterData::get_attack);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "attack"), "set_attack", "get_attack");

    ClassDB::bind_method(D_METHOD("set_defense", "val"), &MonsterData::set_defense);
    ClassDB::bind_method(D_METHOD("get_defense"), &MonsterData::get_defense);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "defense"), "set_defense", "get_defense");

    ClassDB::bind_method(D_METHOD("set_speed", "val"), &MonsterData::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &MonsterData::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "speed"), "set_speed", "get_speed");

    ClassDB::bind_method(D_METHOD("set_current_hp", "val"), &MonsterData::set_current_hp);
    ClassDB::bind_method(D_METHOD("get_current_hp"), &MonsterData::get_current_hp);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "current_hp"), "set_current_hp", "get_current_hp");

    ClassDB::bind_method(D_METHOD("set_rank", "val"), &MonsterData::set_rank);
    ClassDB::bind_method(D_METHOD("get_rank"), &MonsterData::get_rank);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "rank", PROPERTY_HINT_RANGE, "1,4"), "set_rank", "get_rank");

    // --- 特性 ---
    ClassDB::bind_method(D_METHOD("set_ability", "val"), &MonsterData::set_ability);
    ClassDB::bind_method(D_METHOD("get_ability"), &MonsterData::get_ability);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "ability", PROPERTY_HINT_ENUM, "None,Physical Boost,Special Boost"), "set_ability", "get_ability");

    // --- 3Dモデルのパス ---
    ClassDB::bind_method(D_METHOD("set_model_path", "path"), &MonsterData::set_model_path);
    ClassDB::bind_method(D_METHOD("get_model_path"), &MonsterData::get_model_path);
    // ファイル選択ダイアログが出るように PROPERTY_HINT_FILE を指定
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "model_path", PROPERTY_HINT_FILE, "*.tscn,*.scm"), "set_model_path", "get_model_path");

    // --- 技リスト ---
    ClassDB::bind_method(D_METHOD("set_skills", "p_skills"), &MonsterData::set_skills);
    ClassDB::bind_method(D_METHOD("get_skills"), &MonsterData::get_skills);
    
    // エディタ上で SkillData のみを配列に入れられるようにする設定
    // "24/17:SkillData" は Godot 4.x GDExtension の特殊記法で、「Object型(24)かつResource(17)でクラス名がSkillData」を意味します
    ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "skills", PROPERTY_HINT_TYPE_STRING, "24/17:SkillData"), "set_skills", "get_skills");
    ClassDB::bind_method(D_METHOD("set_resource_path", "path"), &MonsterData::set_resource_path);
    ClassDB::bind_method(D_METHOD("get_resource_path"), &MonsterData::get_resource_path);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "resource_path"), "set_resource_path", "get_resource_path");


    // 便利関数
    ClassDB::bind_method(D_METHOD("set_stats", "hp", "atk", "def", "spd"), &MonsterData::set_stats);
}

MonsterData::MonsterData()
{
    monster_name = "Unknown";
    id = "";
    max_hp = 10;
    attack = 1;
    defense = 0;
    speed = 1;
    rank = 1;

    // 初期化
    ability = ABILITY_NONE;
    model_path = "";
}

MonsterData::~MonsterData()
{
}

void MonsterData::set_stats(int p_hp, int p_atk, int p_def, int p_spd)
{
    max_hp = p_hp;
    attack = p_atk;
    defense = p_def;
    speed = p_spd;
}

// --- 既存の実装 ---
void MonsterData::set_monster_name(const String& name) { monster_name = name; }
String MonsterData::get_monster_name() const { return monster_name; }

void MonsterData::set_id(const String& p_id) { id = p_id; }
String MonsterData::get_id() const { return id; }

void MonsterData::set_max_hp(int val) { max_hp = val; }
int MonsterData::get_max_hp() const { return max_hp; }

void MonsterData::set_attack(int val) { attack = val; }
int MonsterData::get_attack() const { return attack; }

void MonsterData::set_defense(int val) { defense = val; }
int MonsterData::get_defense() const { return defense; }

void MonsterData::set_speed(int val) { speed = val; }
int MonsterData::get_speed() const { return speed; }

void MonsterData::set_current_hp(int val) { current_hp = val; }
int MonsterData::get_current_hp() const { return current_hp; }

// ---追加分の実装 ---

void MonsterData::set_ability(int val) { ability = val; }
int MonsterData::get_ability() const { return ability; }

void MonsterData::set_skills(const TypedArray<SkillData>& p_skills) { skills = p_skills; }
TypedArray<SkillData> MonsterData::get_skills() const { return skills; }

void MonsterData::set_model_path(const String& path) { model_path = path; }
String MonsterData::get_model_path() const { return model_path; }

void MonsterData::set_resource_path(const String& path) { resource_path = path; }
String MonsterData::get_resource_path() const { return resource_path; }

void MonsterData::set_rank(int val) { rank = val; }
int MonsterData::get_rank() const { return rank; }