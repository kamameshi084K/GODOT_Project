#include "battle_ui.hpp"
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/input.hpp> // マウス制御用
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void BattleUI::_bind_methods()
{
    // エディタのシグナル接続画面から見えるように登録
    ClassDB::bind_method(D_METHOD("_on_win_button_pressed"), &BattleUI::_on_win_button_pressed);
}

BattleUI::BattleUI()
{
}

BattleUI::~BattleUI()
{
}

void BattleUI::_ready()
{
    // バトル画面ではマウスカーソルが見えないとボタンが押せないので、表示モードにする
    Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
}

void BattleUI::_on_win_button_pressed()
{
    UtilityFunctions::print("Battle Win! Returning to dungeon...");
    
    // ダンジョンに戻る！
    get_tree()->change_scene_to_file("res://dungeon.tscn");
}