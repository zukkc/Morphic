#include "world_save_service.h"

#include "utils/debug_utils.h"

#include <godot_cpp/classes/config_file.hpp>
#include <godot_cpp/classes/dir_access.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace morphic {

static String _normalize_user_path(const String &path) {
  // Keep it simple and consistent: we accept "user://..." or relative-to-user
  // paths.
  if (path.begins_with("user://"))
    return path;
  return String("user://").path_join(path);
}

bool WorldSaveService::ensure_save_directory(const String &save_dir_path) {
  ERR_FAIL_COND_V_MSG(save_dir_path.is_empty(), false,
                      "WorldSaveService: Save directory path is empty");

  const String normalized = _normalize_user_path(save_dir_path);

  Ref<DirAccess> dir = DirAccess::open("user://");
  ERR_FAIL_COND_V_MSG(dir.is_null(), false,
                      "WorldSaveService: Cannot open user://");

  // DirAccess opened on user:// expects relative paths from user://
  String rel = normalized.trim_prefix("user://");

  if (!dir->dir_exists(rel)) {
    Error err = dir->make_dir_recursive(rel);
    ERR_FAIL_COND_V_MSG(
        err != OK, false,
        DebugUtils::format_log(
            "WorldSaveService: Failed creating save dir. Error: %d", err));
  }

  return true;
}

bool WorldSaveService::world_cfg_exists(const String &save_dir_path) const {
  const String normalized = _normalize_user_path(save_dir_path);
  const String cfg_path = normalized.path_join(k_world_config_name);
  return FileAccess::file_exists(cfg_path);
}

WorldSaveService::WorldSaveInfo
WorldSaveService::read_world_cfg(const String &save_dir_path) {
  WorldSaveInfo info;

  const String normalized = _normalize_user_path(save_dir_path);
  info.save_dir = normalized;
  info.world_cfg_path = normalized.path_join(k_world_config_name);
  info.terrain_db_path = normalized.path_join(k_terrain_db_name);

  ERR_FAIL_COND_V_MSG(
      !FileAccess::file_exists(info.world_cfg_path), info,
      "WorldSaveService: world.cfg does not exist (cannot load existing save)");

  Ref<ConfigFile> cfg;
  cfg.instantiate();

  Error err = cfg->load(info.world_cfg_path);
  ERR_FAIL_COND_V_MSG(
      err != OK, info,
      DebugUtils::format_log(
          "WorldSaveService: Failed loading world.cfg. Error: %d", err));

  info.config_version = (int)cfg->get_value("world", "version", 0);
  info.seed = (int)cfg->get_value("world", "seed", 0);
  info.generator_signature =
      (String)cfg->get_value("world", "generator_signature", "");

  // This is load-existing path
  info.is_new = false;

  // Hard guard: seed == 0 often indicates “oops created default”. Still allow,
  // but warn.
  if (info.seed == 0) {
    WARN_PRINT("WorldSaveService: Loaded save with seed=0. If this was "
               "unintended, fix world.cfg creation logic.");
  }

  return info;
}

WorldSaveService::WorldSaveInfo
WorldSaveService::write_world_cfg_new(const String &save_dir_path, int seed) {
  WorldSaveInfo info;

  const String normalized = _normalize_user_path(save_dir_path);
  info.save_dir = normalized;
  info.world_cfg_path = normalized.path_join(k_world_config_name);
  info.terrain_db_path = normalized.path_join(k_terrain_db_name);

  ERR_FAIL_COND_V_MSG(seed == 0, info,
                      "WorldSaveService: Refusing to create new save with "
                      "seed=0 (use a real seed)");

  // New save must not exist (as per your requirement)
  ERR_FAIL_COND_V_MSG(FileAccess::file_exists(info.world_cfg_path), info,
                      "WorldSaveService: Save already exists (world.cfg "
                      "present). Use load_existing instead.");

  Ref<ConfigFile> cfg;
  cfg.instantiate();

  cfg->set_value("world", "version", k_world_config_version);
  cfg->set_value("world", "seed", seed);

  // Optional: terrain will validate this; here we just store placeholder.
  // You can set this from outside if you want, but keep the save module dumb.
  cfg->set_value("world", "generator_signature", "");

  Error err = cfg->save(info.world_cfg_path);
  ERR_FAIL_COND_V_MSG(
      err != OK, info,
      DebugUtils::format_log(
          "WorldSaveService: Failed saving world.cfg. Error: %d", err));

  info.config_version = k_world_config_version;
  info.seed = seed;
  info.is_new = true;

  return info;
}

WorldSaveService::WorldSaveInfo
WorldSaveService::load_existing(const String &save_dir_path) {
  ERR_FAIL_COND_V_MSG(save_dir_path.is_empty(), WorldSaveInfo(),
                      "WorldSaveService: save_dir_path is empty");

  // For existing save, directory should exist too. If not, fail clearly.
  ERR_FAIL_COND_V_MSG(
      !world_cfg_exists(save_dir_path), WorldSaveInfo(),
      "WorldSaveService: No save found (world.cfg missing). Use create_new.");

  return read_world_cfg(save_dir_path);
}

WorldSaveService::WorldSaveInfo
WorldSaveService::create_new(const String &save_dir_path, int seed) {
  ERR_FAIL_COND_V_MSG(save_dir_path.is_empty(), WorldSaveInfo(),
                      "WorldSaveService: save_dir_path is empty");

  ERR_FAIL_COND_V_MSG(
      !ensure_save_directory(save_dir_path), WorldSaveInfo(),
      "WorldSaveService: Failed creating/ensuring save directory.");

  // Must not exist already
  ERR_FAIL_COND_V_MSG(
      world_cfg_exists(save_dir_path), WorldSaveInfo(),
      "WorldSaveService: Save already exists. Use load_existing.");

  return write_world_cfg_new(save_dir_path, seed);
}

Dictionary WorldSaveService::to_dict(const WorldSaveInfo &info) const {
  Dictionary d;
  d["config_version"] = info.config_version;
  d["seed"] = info.seed;
  d["is_new"] = info.is_new;
  d["save_dir"] = info.save_dir;
  d["world_cfg_path"] = info.world_cfg_path;
  d["terrain_db_path"] = info.terrain_db_path;
  d["generator_signature"] = info.generator_signature;
  return d;
}

Dictionary WorldSaveService::load_existing_dict(const String &save_dir_path) {
  return to_dict(load_existing(save_dir_path));
}

Dictionary WorldSaveService::create_new_dict(const String &save_dir_path,
                                             int seed) {
  return to_dict(create_new(save_dir_path, seed));
}

} // namespace morphic
