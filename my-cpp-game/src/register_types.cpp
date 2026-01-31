#include "register_types.h"
#include "gdexample.h" // 自作クラスをインクルード
#include "player.hpp"
#include "enemy.hpp"
#include "warp_area.hpp"
#include "battle_ui.hpp"
#include "game_manager.hpp"
#include "hud.hpp"
#include "enemy_spawner.hpp"
#include "npc.hpp"
#include "dialogue_box.hpp"
#include "title_screen.hpp"
#include "monster_data.hpp"
#include "world.hpp" 
#include "town.hpp"  

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

void initialize_gdexample_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
    // ここで自作クラスを登録！
    ClassDB::register_class<GDExample>();
    ClassDB::register_class<Player>();
    ClassDB::register_class<Enemy>();   // 敵クラスを登録
    ClassDB::register_class<WarpArea>(); // ワープエリアクラスを登録
    ClassDB::register_class<BattleUI>(); // バトルUIクラスを登録
    ClassDB::register_class<GameManager>(); // ゲームマネージャークラスを登録
    ClassDB::register_class<HUD>(); // HUDクラスを登録
    ClassDB::register_class<EnemySpawner>(); // 敵スポーン管理クラスを登録
    ClassDB::register_class<NPC>(); // NPCクラスを登録
    ClassDB::register_class<DialogueBox>(); // ダイアログボックスクラスを登録
    ClassDB::register_class<TitleScreen>(); // タイトル画面クラスを登録
    ClassDB::register_class<MonsterData>(); // モンスター情報クラスを登録
    ClassDB::register_class<World>(); // ワールドクラスを登録
    ClassDB::register_class<Town>(); // タウンクラスを登録
}

void uninitialize_gdexample_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C" 
{
    // 初期化関数のエントリーポイント
    GDExtensionBool GDE_EXPORT gdexample_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        init_obj.register_initializer(initialize_gdexample_module);
        init_obj.register_terminator(uninitialize_gdexample_module);
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}