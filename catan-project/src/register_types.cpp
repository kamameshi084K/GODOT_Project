#include "register_types.h"

// 登録する自作クラスのヘッダー
#include "hex_tile.hpp"
#include "board.hpp"
#include "catan_game.hpp"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

using namespace godot;

/**
 * @brief モジュールの初期化処理
 * @param p_level 初期化のフェーズ。SCENEレベルでクラスの登録を行います。
 */
void initialize_catan_module(ModuleInitializationLevel p_level) 
{
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) 
    {
        return;
    }

    // GodotエディタおよびエンジンにHexTileクラスを登録
    ClassDB::register_class<HexTile>();
    // GodotエディタおよびエンジンにBoardクラスを登録
    ClassDB::register_class<Board>();
    // GodotエディタおよびエンジンにCatanGameクラスを登録
    ClassDB::register_class<CatanGame>();
}

/**
 * @brief モジュールの終了処理
 * @param p_level 終了のフェーズ。
 */
void uninitialize_catan_module(ModuleInitializationLevel p_level) 
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
     * @return 初期化が成功したかどうか
     */
    GDExtensionBool GDE_EXPORT gdexample_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) 
    {
        godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

        // 初期化関数と終了関数の登録
        init_obj.register_initializer(initialize_catan_module);
        init_obj.register_terminator(uninitialize_catan_module);
        
        // シーンレベルでの初期化を最小レベルとして設定
        init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

        return init_obj.init();
    }
}