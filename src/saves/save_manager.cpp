#include "save_manager.h"

#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace morphic {

void SaveManager::_set_active_from_info(const Dictionary &info) {
  _last_save_info = info;
  _active_save_dir = String(info.get("save_dir", ""));
  _has_active_save = !_active_save_dir.is_empty();
}

Dictionary SaveManager::load_existing(const String &save_dir_path) {
  // WorldSaveService zrobi twarde walidacje (cfg istnieje itd.)
  Dictionary info = _service.load_existing_dict(save_dir_path);
  _set_active_from_info(info);

  emit_signal("save_loaded", info);
  return info;
}

Dictionary SaveManager::create_new(const String &save_dir_path, int seed) {
  Dictionary info = _service.create_new_dict(save_dir_path, seed);
  _set_active_from_info(info);

  emit_signal("save_created", info);
  return info;
}

void SaveManager::clear_active_save() {
  _has_active_save = false;
  _active_save_dir = "";
  _last_save_info.clear();

  emit_signal("save_cleared");
}

void SaveManager::_bind_methods() {
  // Methods
  ClassDB::bind_method(D_METHOD("load_existing", "save_dir_path"), &SaveManager::load_existing);
  ClassDB::bind_method(D_METHOD("create_new", "save_dir_path", "seed"), &SaveManager::create_new);
  ClassDB::bind_method(D_METHOD("clear_active_save"), &SaveManager::clear_active_save);

  // Getters
  ClassDB::bind_method(D_METHOD("has_active_save"), &SaveManager::has_active_save);
  ClassDB::bind_method(D_METHOD("get_active_save_dir"), &SaveManager::get_active_save_dir);
  ClassDB::bind_method(D_METHOD("get_last_save_info"), &SaveManager::get_last_save_info);

  // Signals
  ADD_SIGNAL(MethodInfo("save_loaded", PropertyInfo(Variant::DICTIONARY, "info")));
  ADD_SIGNAL(MethodInfo("save_created", PropertyInfo(Variant::DICTIONARY, "info")));
  ADD_SIGNAL(MethodInfo("save_cleared"));
}

} // namespace morphic
