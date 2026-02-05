#pragma once

#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace morphic {

class WorldSaveService {

public:
  struct WorldSaveInfo {
    int config_version = 0;
    int seed = 0;
    bool is_new = false;

    // paths derived from save_dir
    String save_dir;
    String world_cfg_path;
    String terrain_db_path;

    // optional metadata, terrain can use it to validate generator compatibility
    String generator_signature; // e.g. "graph_hash:abcd..." or "v1"
  };

  WorldSaveService() = default;
  ~WorldSaveService() = default;

  // Case 1: Load existing save (no seed argument)
  WorldSaveInfo load_existing(const String &save_dir_path);

  // Case 2: Create new save (seed required). Fails if save already exists.
  WorldSaveInfo create_new(const String &save_dir_path, int seed);

  // Convenience for GDScript / Godot calls (returns Dictionary)
  Dictionary load_existing_dict(const String &save_dir_path);
  Dictionary create_new_dict(const String &save_dir_path, int seed);

private:
  static constexpr int k_world_config_version = 1;
  static constexpr const char *k_world_config_name = "world.cfg";
  static constexpr const char *k_terrain_db_name = "terrain.sqlite";

  bool ensure_save_directory(const String &save_dir_path);
  bool world_cfg_exists(const String &save_dir_path) const;

  WorldSaveInfo read_world_cfg(const String &save_dir_path);
  WorldSaveInfo write_world_cfg_new(const String &save_dir_path, int seed);

  Dictionary to_dict(const WorldSaveInfo &info) const;
};

} // namespace morphic
