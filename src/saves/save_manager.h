#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

#include "world_save_service.h"

using namespace godot;

namespace morphic {

class SaveManager : public Node {
  GDCLASS(SaveManager, Node);

public:
  SaveManager() = default;
  ~SaveManager() = default;

  // API dla menu / gry
  Dictionary load_existing(const String &save_dir_path);
  Dictionary create_new(const String &save_dir_path, int seed);

  // Kontekst
  bool has_active_save() const { return _has_active_save; }
  String get_active_save_dir() const { return _active_save_dir; }
  Dictionary get_last_save_info() const { return _last_save_info; }

  void clear_active_save();

protected:
  static void _bind_methods();

private:
  bool _has_active_save = false;
  String _active_save_dir;
  Dictionary _last_save_info;

  // Jedna instancja serwisu I/O – tylko tu.
  WorldSaveService _service;

  void _set_active_from_info(const Dictionary &info);
};

} // namespace morphic

