#pragma once

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

// モジュール初期化時のコールバック関数
void initialize_catan_module(ModuleInitializationLevel p_level);
// モジュール終了時のコールバック関数
void uninitialize_catan_module(ModuleInitializationLevel p_level);