#include "npc.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

void NPC::_bind_methods()
{
    // interact関数をGodot側（Player）から呼べるように登録
    ClassDB::bind_method(D_METHOD("interact"), &NPC::interact);

    // セリフプロパティの登録
    ClassDB::bind_method(D_METHOD("set_dialogue_text", "p_text"), &NPC::set_dialogue_text);
    ClassDB::bind_method(D_METHOD("get_dialogue_text"), &NPC::get_dialogue_text);
    ADD_PROPERTY(PropertyInfo(Variant::STRING, "dialogue_text", PROPERTY_HINT_MULTILINE_TEXT), "set_dialogue_text", "get_dialogue_text");
}

NPC::NPC()
{
    dialogue_text = "Hello!";
}

NPC::~NPC()
{
}

void NPC::_ready()
{
    // 必要なら重力処理などをここに追加
}

// プレイヤーが話しかけた時に呼ばれる関数
void NPC::interact()
{
    UtilityFunctions::print("NPC says: ", dialogue_text);
    
    // シーン内にある "dialogue_ui" グループの最初のノードを探す
    Node* ui_node = get_tree()->get_first_node_in_group("dialogue_ui");
    
    if (ui_node)
    {
        // もし見つかったら、show_message関数を呼ぶ
        // (callメソッドを使うと、型を知らなくても関数名だけで呼べます)
        ui_node->call("show_message", dialogue_text);
    }
    else
    {
        UtilityFunctions::print("Error: DialogueBox not found in scene!");
    }
}

// --- セッター・ゲッター ---

void NPC::set_dialogue_text(const String& p_text)
{
    dialogue_text = p_text;
}

String NPC::get_dialogue_text() const
{
    return dialogue_text;
}