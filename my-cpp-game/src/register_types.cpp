#include "register_types.h"
#include "gdexample.h" // 自作クラスをインクルード
#include "player.hpp"
#include "enemy.hpp"
#include "portal.hpp"
#include "battle_ui.hpp"
#include "game_manager.hpp"

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
    ClassDB::register_class<Portal>(); // ポータルクラスを登録
    ClassDB::register_class<BattleUI>(); // バトルUIクラスを登録
    ClassDB::register_class<GameManager>(); // ゲームマネージャークラスを登録
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