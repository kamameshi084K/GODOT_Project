#include "register_types.h"
#include "follower.hpp"
#include "Player.hpp"

#include <gdextension_interface.h>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>

// 今後作成するキャラクターのヘッダーをここにインクルードします
// #include "follower.h"

using namespace godot;

void initialize_pikumin_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }

    // 今後作成するクラスをGodotに登録する処理をここに書きます
    ClassDB::register_class<Follower>();
    ClassDB::register_class<Player>();
}

void uninitialize_pikumin_module(ModuleInitializationLevel p_level) {
    if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
        return;
    }
}

extern "C" {
// .gdextensionファイルの "entry_symbol" で指定した名前と完全に一致させる必要があります
GDExtensionBool GDE_EXPORT gdexample_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address, const GDExtensionClassLibraryPtr p_library, GDExtensionInitialization *r_initialization) {
    godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library, r_initialization);

    init_obj.register_initializer(initialize_pikumin_module);
    init_obj.register_terminator(uninitialize_pikumin_module);
    init_obj.set_minimum_library_initialization_level(MODULE_INITIALIZATION_LEVEL_SCENE);

    return init_obj.init();
}
}