#include "battle_scene.hpp"
#include "game_manager.hpp"
#include "monster_data.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

void BattleScene::_bind_methods()
{
    ClassDB::bind_method(D_METHOD("_rpc_spawn_enemy", "monster_name"), &BattleScene::_rpc_spawn_enemy);
}

BattleScene::BattleScene()
{
    player_spawn_pos = nullptr;
    enemy_spawn_pos = nullptr;
    // RPC設定：相手の画面で実行させる
    Dictionary rpc_config_spawn;
    rpc_config_spawn["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER; // 誰からでも受け取る
    rpc_config_spawn["call_local"] = false; // 自分には送らなくていい（自分で出すから）
    rpc_config_spawn["transfer_mode"] = MultiplayerPeer::TRANSFER_MODE_RELIABLE; // 確実に届ける
    rpc_config("_rpc_spawn_enemy", rpc_config_spawn);
}

BattleScene::~BattleScene()
{
}

void BattleScene::_ready()
{
    // マーカーを取得
    Node* p_node = find_child("PlayerSpawnPos");
    if (p_node) player_spawn_pos = Object::cast_to<Marker3D>(p_node);

    Node* e_node = find_child("EnemySpawnPos");
    if (e_node) enemy_spawn_pos = Object::cast_to<Marker3D>(e_node);

    // --- 自分のモンスターを出現させる ---
    GameManager* gm = GameManager::get_singleton();
    if (gm)
    {
        // 手持ちの先頭（リーダー）を取得
        TypedArray<MonsterData> party = gm->get_party();
        if (party.size() > 0)
        {
            Ref<MonsterData> leader = party[0];
            String name = leader->get_monster_name();
            
            // 1. 自分の画面の「プレイヤー位置」に出す
            String path = _get_model_path_by_name(name);
            Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
            if (scene.is_valid())
            {
                Node* instance = scene->instantiate();
                if (player_spawn_pos) player_spawn_pos->add_child(instance);
                UtilityFunctions::print("Spawned my monster: ", name);
            }

            // 2. 相手の画面へ「俺のモンスターはこれだ！」と通知する
            // これにより、相手の画面では _rpc_spawn_enemy が実行され、「敵位置」にこれが出る
            rpc("_rpc_spawn_enemy", name);
        }
    }
}

// 相手から送られてくる関数
void BattleScene::_rpc_spawn_enemy(const String& monster_name)
{
    UtilityFunctions::print("Opponent sent monster: ", monster_name);

    String path = _get_model_path_by_name(monster_name);
    Ref<PackedScene> scene = ResourceLoader::get_singleton()->load(path);
    
    if (scene.is_valid() && enemy_spawn_pos)
    {
        Node* instance = scene->instantiate();
        enemy_spawn_pos->add_child(instance);
    }
}

// 名前からファイルパスへの変換（辞書代わり）
String BattleScene::_get_model_path_by_name(const String& name)
{
    // ※ 注意: 実際のファイルパスに合わせて書き換えてください！
    // assetsフォルダにある .glb や .tscn を指定します
    
    if (name == "Flame Lizard") return "res://assets/Birb.gltf"; // 仮: トカゲのモデルがあればそれに変える
    if (name == "Aqua Turtle")  return "res://assets/Birb.gltf"; // 仮
    if (name == "Leaf Cat")     return "res://assets/Birb.gltf"; // 仮
    
    return "res://assets/Birb.gltf"; // デフォルト（あの鳥）
}