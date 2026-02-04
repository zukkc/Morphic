#include "register_types.h"
#include "network_manager.h"
#include "player/local_player_controller.h"
#include "player/player.h"
#include "ui/main_menu.h"
#include "world/player_spawner.h"
#include "world/world.h"
#include "world_loader.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/core/defs.hpp>
#include <godot_cpp/godot.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

// Tutaj będziesz rejestrować swoje klasy w przyszłości
void initialize_morphic_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
  ClassDB::register_class<morphic::NetworkManager>();
  ClassDB::register_class<morphic::World>();
  ClassDB::register_class<morphic::PlayerSpawner>();
  ClassDB::register_class<morphic::Player>();
  ClassDB::register_class<morphic::LocalPlayerController>();
  ClassDB::register_class<morphic::MainMenu>();
  ClassDB::register_class<morphic::WorldLoader>();
  UtilityFunctions::print("morphic_core loaded!");
}

void uninitialize_morphic_module(ModuleInitializationLevel p_level) {
  if (p_level != MODULE_INITIALIZATION_LEVEL_SCENE) {
    return;
  }
}

extern "C" {
GDExtensionBool GDE_EXPORT
morphic_library_init(GDExtensionInterfaceGetProcAddress p_get_proc_address,
                     const GDExtensionClassLibraryPtr p_library,
                     GDExtensionInitialization *r_initialization) {
  godot::GDExtensionBinding::InitObject init_obj(p_get_proc_address, p_library,
                                                 r_initialization);

  init_obj.register_initializer(initialize_morphic_module);
  init_obj.register_terminator(uninitialize_morphic_module);
  init_obj.set_minimum_library_initialization_level(
      MODULE_INITIALIZATION_LEVEL_SCENE);

  return init_obj.init();
}
}
