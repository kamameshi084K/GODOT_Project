#include "monster_data.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void MonsterData::_bind_methods()
{
    // 名前
    ClassDB::bind_method(D_METHOD("set_monster_name", "name"), &MonsterData::set_monster_name);
    ClassDB::bind_method(D_METHOD("get_monster_name"), &MonsterData::get_monster_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "monster_name"), "set_monster_name", "get_monster_name");

    // ID
    ClassDB::bind_method(D_METHOD("set_id", "id"), &MonsterData::set_id);
    ClassDB::bind_method(D_METHOD("get_id"), &MonsterData::get_id);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "id"), "set_id", "get_id");

    // HP
    ClassDB::bind_method(D_METHOD("set_max_hp", "val"), &MonsterData::set_max_hp);
    ClassDB::bind_method(D_METHOD("get_max_hp"), &MonsterData::get_max_hp);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "max_hp"), "set_max_hp", "get_max_hp");

    // 攻撃力
    ClassDB::bind_method(D_METHOD("set_attack", "val"), &MonsterData::set_attack);
    ClassDB::bind_method(D_METHOD("get_attack"), &MonsterData::get_attack);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "attack"), "set_attack", "get_attack");

    // 防御力
    ClassDB::bind_method(D_METHOD("set_defense", "val"), &MonsterData::set_defense);
    ClassDB::bind_method(D_METHOD("get_defense"), &MonsterData::get_defense);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "defense"), "set_defense", "get_defense");

    // 素早さ
    ClassDB::bind_method(D_METHOD("set_speed", "val"), &MonsterData::set_speed);
    ClassDB::bind_method(D_METHOD("get_speed"), &MonsterData::get_speed);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "speed"), "set_speed", "get_speed");

    // 便利関数（GDScriptから呼べるようにするなら登録）
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

// --- セッター・ゲッターの実装 ---

void MonsterData::set_monster_name(const String& name)
{
    monster_name = name;
}

String MonsterData::get_monster_name() const
{
    return monster_name;
}

void MonsterData::set_id(const String& p_id)
{
    id = p_id;
}

String MonsterData::get_id() const
{
    return id;
}

void MonsterData::set_max_hp(int val)
{
    max_hp = val;
}

int MonsterData::get_max_hp() const
{
    return max_hp;
}

void MonsterData::set_attack(int val)
{
    attack = val;
}

int MonsterData::get_attack() const
{
    return attack;
}

void MonsterData::set_defense(int val)
{
    defense = val;
}

int MonsterData::get_defense() const
{
    return defense;
}

void MonsterData::set_speed(int val)
{
    speed = val;
}

int MonsterData::get_speed() const
{
    return speed;
}