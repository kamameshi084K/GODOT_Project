#include "register_types.h"
#include "unit_core.hpp"
#include "turn_manager_core.hpp"
#include "skill_data.hpp"
#include "arena_grid_core.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

/**
 * @brief モジュールの初期化処理
 * @param p_level 初期化のフェーズ。SCENEレベルでクラスの登録を行います。
 */
void initialize_tactics_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }
    GDREGISTER_CLASS(UnitCore);
    GDREGISTER_CLASS(TurnManagerCore);
    GDREGISTER_CLASS(SkillData);
    GDREGISTER_CLASS(ArenaGridCore);
    // ここに今後作成する自作クラス（CombatMathやCharacterStatsなど）を登録します
    // 例: ClassDB::register_class<CombatMath>();
}

/**
 * @brief モジュールの終了処理
 * @param p_level 終了のフェーズ。
 */
void uninitialize_tactics_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE)
    {
        return;
    }
}

extern "C" 
{
    /**
     * @brief GDExtensionのライブラリ初期化エントリーポイント
     * .gdextension 内の entry_symbol = "tactics_core_init" と一致させる必要があります
     */
    GDExtensionBool GDE_EXPORT tactics_core_init(
        GDExtensionInterfaceGetProcAddress p_get_proc_address, 
        const GDExtensionClassLibraryPtr p_library, 
        GDExtensionInitialization *r_initialization
    ) 
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        // 初期化関数と終了関数の登録
        init_obj.register_initializer(initialize_tactics_module);
        init_obj.register_terminator(uninitialize_tactics_module);
        
        // シーンレベルでの初期化を最小レベルとして設定
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}