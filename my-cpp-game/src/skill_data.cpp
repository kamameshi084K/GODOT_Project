#include "skill_data.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

SkillData::SkillData()
{
    skill_name = "New Skill";
    hand_type = HAND_ROCK;
    attack_type = ATTACK_PHYSICAL;
    power = 10;
    
    // ★追加: 新しい変数の初期化
    animation_name = "Idle"; // デフォルトは棒立ちにしておく
    is_physical = true;      // デフォルトは物理（移動する）にしておく
}

SkillData::~SkillData()
{
}

void SkillData::set_skill_name(const String& name) { skill_name = name; }
String SkillData::get_skill_name() const { return skill_name; }

void SkillData::set_hand_type(int type) { hand_type = type; }
int SkillData::get_hand_type() const { return hand_type; }

void SkillData::set_attack_type(int type) { attack_type = type; }
int SkillData::get_attack_type() const { return attack_type; }

void SkillData::set_power(int value) { power = value; }
int SkillData::get_power() const { return power; }

// ★追加: アニメーション名の実装
void SkillData::set_animation_name(const String& name) { animation_name = name; }
String SkillData::get_animation_name() const { return animation_name; }

// ★追加: 物理フラグの実装
void SkillData::set_is_physical(bool is_phys) { is_physical = is_phys; }
bool SkillData::get_is_physical() const { return is_physical; }

void SkillData::_bind_methods()
{
    // 技名
    ClassDB::bind_method(D_METHOD("set_skill_name", "name"), &SkillData::set_skill_name);
    ClassDB::bind_method(D_METHOD("get_skill_name"), &SkillData::get_skill_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "skill_name"), "set_skill_name", "get_skill_name");

    // 手のタイプ
    ClassDB::bind_method(D_METHOD("set_hand_type", "type"), &SkillData::set_hand_type);
    ClassDB::bind_method(D_METHOD("get_hand_type"), &SkillData::get_hand_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "hand_type", PROPERTY_HINT_ENUM, "Rock,Scissors,Paper"), "set_hand_type", "get_hand_type");

    // 攻撃タイプ
    ClassDB::bind_method(D_METHOD("set_attack_type", "type"), &SkillData::set_attack_type);
    ClassDB::bind_method(D_METHOD("get_attack_type"), &SkillData::get_attack_type);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "attack_type", PROPERTY_HINT_ENUM, "Physical,Special"), "set_attack_type", "get_attack_type");

    // 威力
    ClassDB::bind_method(D_METHOD("set_power", "value"), &SkillData::set_power);
    ClassDB::bind_method(D_METHOD("get_power"), &SkillData::get_power);
    ADD_PROPERTY(PropertyInfo(Variant::INT, "power", PROPERTY_HINT_RANGE, "0,999"), "set_power", "get_power");

    // ★追加: アニメーション名の登録
    ClassDB::bind_method(D_METHOD("set_animation_name", "name"), &SkillData::set_animation_name);
    ClassDB::bind_method(D_METHOD("get_animation_name"), &SkillData::get_animation_name);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "animation_name"), "set_animation_name", "get_animation_name");

    // ★追加: 物理フラグの登録
    ClassDB::bind_method(D_METHOD("set_is_physical", "is_phys"), &SkillData::set_is_physical);
    ClassDB::bind_method(D_METHOD("get_is_physical"), &SkillData::get_is_physical);
    ADD_PROPERTY(PropertyInfo(Variant::BOOL, "is_physical"), "set_is_physical", "get_is_physical");
}