#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// 静的変数の初期化
GameManager *GameManager::singleton = nullptr;

void GameManager::_bind_methods()
{
    // メソッドのバインド
    ClassDB::bind_method(D_METHOD("set_last_player_position", "pos"), &GameManager::set_last_player_position);
    ClassDB::bind_method(D_METHOD("get_last_player_position"), &GameManager::get_last_player_position);
    
    // --- is_returning_from_battle のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("set_is_returning_from_battle", "value"), &GameManager::set_is_returning_from_battle);
    ClassDB::bind_method(D_METHOD("get_is_returning_from_battle"), &GameManager::get_is_returning_from_battle);

    // --- next_enemy_scene_path のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("set_current_enemy_id", "id"), &GameManager::set_current_enemy_id);
    ClassDB::bind_method(D_METHOD("get_current_enemy_id"), &GameManager::get_current_enemy_id);
    
    // --- defeated_enemies のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("add_defeated_enemy", "id"), &GameManager::add_defeated_enemy);
    ClassDB::bind_method(D_METHOD("is_enemy_defeated", "id"), &GameManager::is_enemy_defeated);

    // --- next_enemy_scene_path のセッター・ゲッター ---
    ClassDB::bind_method(D_METHOD("get_player_attack"), &GameManager::get_player_attack);
    ClassDB::bind_method(D_METHOD("get_player_defense"), &GameManager::get_player_defense);
}

GameManager::GameManager()
{
    singleton = this;
    is_returning_from_battle = false;
    current_enemy_id = ""; // 初期化
    next_enemy_name = "Enemy";
    next_enemy_max_hp = 3;
    next_enemy_attack = 1;
    next_enemy_defense = 0;

    player_attack = 5;  // 例: 攻撃力5
    player_defense = 0; // 例: 防御力0
}

void GameManager::set_next_enemy_stats(const String &name, int hp, int attack)
{
    next_enemy_name = name;
    next_enemy_max_hp = hp;
    next_enemy_attack = attack;
}

void GameManager::set_player_stats(int attack, int defense)
{
    player_attack = attack;
    player_defense = defense;
}

GameManager::~GameManager()
{
    if (singleton == this)
    {
        singleton = nullptr;
    }
}

GameManager *GameManager::get_singleton()
{
    return singleton;
}

void GameManager::set_last_player_position(const Vector3 &pos)
{
    last_player_position = pos;
}

Vector3 GameManager::get_last_player_position() const
{
    return last_player_position;
}

void GameManager::set_is_returning_from_battle(bool value)
{
    is_returning_from_battle = value;
}

bool GameManager::get_is_returning_from_battle() const
{
    return is_returning_from_battle;
}

void GameManager::set_next_enemy_scene_path(const String &path)
{
    next_enemy_scene_path = path;
}

String GameManager::get_next_enemy_scene_path() const
{ 
    return next_enemy_scene_path;
}

void GameManager::set_current_enemy_id(const String &id)
{
    current_enemy_id = id;
}

String GameManager::get_current_enemy_id() const
{
    return current_enemy_id;
}

void GameManager::add_defeated_enemy(const String &id)
{
    // まだリストになければ追加する
    if (!defeated_enemies.has(id))
    {
        defeated_enemies.append(id);
    }
}

bool GameManager::is_enemy_defeated(const String &id) const
{
    return defeated_enemies.has(id);
}

String GameManager::get_next_enemy_name() const
{
    return next_enemy_name;

}
int GameManager::get_next_enemy_max_hp() const
{
    return next_enemy_max_hp;
}

int GameManager::get_next_enemy_attack() const
{
    return next_enemy_attack;
}

int GameManager::get_player_attack() const
{
    return player_attack;
}
int GameManager::get_player_defense() const
{
    return player_defense;
}

int GameManager::get_next_enemy_defense() const
{
    return next_enemy_defense;
}
void GameManager::set_next_enemy_defense(int def)
{
    next_enemy_defense = def;
}