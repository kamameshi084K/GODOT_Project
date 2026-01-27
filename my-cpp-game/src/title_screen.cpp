#include "title_screen.hpp"
#include "game_manager.hpp"

#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

void TitleScreen::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_on_start_button_pressed"), &TitleScreen::_on_start_button_pressed);
    ClassDB::bind_method(D_METHOD("_on_quit_button_pressed"), &TitleScreen::_on_quit_button_pressed);
}

TitleScreen::TitleScreen()
{
}

TitleScreen::~TitleScreen()
{
}

void TitleScreen::_on_start_button_pressed()
{
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        // 0:炎, 1:水, 2:草 (GameManagerの実装に合わせて)
        gm->select_starter_monster(0); 
    }
    // ゲーム開始！町へ移動します
    get_tree()->change_scene_to_file("res://town.tscn");
}

void TitleScreen::_on_quit_button_pressed()
{
    // ゲーム終了
    get_tree()->quit();
}