#include "world.hpp"
#include "game_manager.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

World::World()
{
    time_label = nullptr;
}
World::~World() {}

void World::_ready()
{
    // CanvasLayer/TimeLabel という名前のノードを探す
    Node* node = find_child("TimeLabel", true, false);
    if (node) time_label = Object::cast_to<Label>(node);
}

void World::_process(double delta)
{
    GameManager* gm = GameManager::get_singleton();
    if (gm && time_label)
    {
        float time = gm->get_time_remaining();
        // 0秒未満は0と表示
        if (time < 0) 
        {
            time = 0;
        }
        time_label->set_text("Time: " + String::num((int)time));
    }
}