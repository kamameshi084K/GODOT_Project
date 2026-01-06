#include "game_manager.hpp"
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// 静的変数の初期化
GameManager *GameManager::singleton = nullptr;

/**
 * @brief Godot にメソッドを登録する
 * @return void
 * @note _bind_methods は protected メソッドとして宣言されている必要があります
 */
void GameManager::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("set_last_player_position", "pos"), &GameManager::set_last_player_position);
    ClassDB::bind_method(D_METHOD("get_last_player_position"), &GameManager::get_last_player_position);
    
    ClassDB::bind_method(D_METHOD("set_is_returning_from_battle", "value"), &GameManager::set_is_returning_from_battle);
    ClassDB::bind_method(D_METHOD("get_is_returning_from_battle"), &GameManager::get_is_returning_from_battle);
}

/**
 * @brief コンストラクタ（初期化）
 * @note シングルトンとして振る舞うため、インスタンス生成時に singleton ポインタを設定します
 */
GameManager::GameManager()
{
    singleton = this; // 生まれた瞬間に「私が管理者だ」と名乗り出る
    is_returning_from_battle = false;
    last_player_position = Vector3(0, 0, 0);
}

/**
 * @brief デストラクタ
 * @note シングルトンが破棄されるときに singleton ポインタをクリアします
 */
GameManager::~GameManager()
{
    if (singleton == this)
    {
        singleton = nullptr;
    }
}

/**
 * @brief シングルトンインスタンスを取得する静的メソッド
 * @return GameManager* シングルトンインスタンスへのポインタ
 */
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