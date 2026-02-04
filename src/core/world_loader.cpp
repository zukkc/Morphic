#include "world_loader.h"

#include "utils/debug_utils.h"

#include "godot_cpp/classes/config_file.hpp"
#include "godot_cpp/classes/dir_access.hpp"
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/object.hpp"
#include "godot_cpp/classes/packed_scene.hpp"
#include "godot_cpp/classes/voxel_generator_graph.hpp"
#include "godot_cpp/classes/voxel_graph_function.hpp"
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/voxel_stream_sq_lite.hpp>
#include <godot_cpp/classes/voxel_terrain.hpp>
#include <godot_cpp/classes/window.hpp>

using namespace godot;

namespace morphic {

namespace {
constexpr int k_world_config_version = 1;
const char *k_world_config_name = "world.cfg";
}

WorldLoader::WorldLoader() {}
WorldLoader::~WorldLoader() {}

bool WorldLoader::ensure_save_directory(const String &save_dir_path) {
  ERR_FAIL_COND_V_MSG(save_dir_path.is_empty(), false,
                      "WorldLoader: Save directory path is empty");

  Ref<DirAccess> dir = DirAccess::open("user://");
  ERR_FAIL_COND_V_MSG(dir.is_null(), false,
                      "WorldLoader: Nie można otworzyć user://");

  String dir_path = save_dir_path;
  if (dir_path.begins_with("user://")) {
    dir_path = dir_path.trim_prefix("user://");
  }

  if (!dir->dir_exists(dir_path)) {
    LOG("WorldLoader: Folder zapisu nie istnieje. Tworzenie: %s",
        save_dir_path);
    Error err = dir->make_dir_recursive(dir_path);

    ERR_FAIL_COND_V_MSG(
        err != OK, false,
        DebugUtils::format_log(
            "WorldLoader: Błąd przy tworzeniu folderu! Kod błędu: %d", err));
  }

  return true;
}

bool WorldLoader::prepare_world_config(const String &save_dir_path,
                                       bool use_seed, int seed, int &out_seed) {
  ERR_FAIL_COND_V_MSG(save_dir_path.is_empty(), false,
                      "WorldLoader: Save directory path is empty");

  const String config_path = save_dir_path.path_join(k_world_config_name);
  Ref<ConfigFile> cfg;
  cfg.instantiate();

  if (FileAccess::file_exists(config_path)) {
    Error err = cfg->load(config_path);
    ERR_FAIL_COND_V_MSG(
        err != OK, false,
        DebugUtils::format_log(
            "WorldLoader: Failed loading world.cfg. Error: %d", err));
  }

  const int existing_seed = (int)cfg->get_value("world", "seed", 0);
  out_seed = use_seed ? seed : existing_seed;

  cfg->set_value("world", "version", k_world_config_version);
  cfg->set_value("world", "seed", out_seed);

  Error err = cfg->save(config_path);
  ERR_FAIL_COND_V_MSG(
      err != OK, false,
      DebugUtils::format_log("WorldLoader: Failed saving world.cfg. Error: %d",
                             err));

  return true;
}

void WorldLoader::load_game(const String &save_path, bool is_server,
                            bool use_seed, int seed) {
  int final_seed = 0;
  if (is_server) {
    ERR_FAIL_COND_MSG(
        !ensure_save_directory(save_path),
        "WorldLoader: Krytyczny błąd systemu plików. Anulowanie ładowania.");
    ERR_FAIL_COND_MSG(
        !prepare_world_config(save_path, use_seed, seed, final_seed),
        "WorldLoader: Nie udało się utworzyć world.cfg. Anulowanie ładowania.");
  }

  Ref<PackedScene> world_res =
      ResourceLoader::get_singleton()->load(world_scene_path);
  ERR_FAIL_COND_MSG(world_res.is_null(),
                    "WorldLoader: Nie znaleziono world.tscn");

  Node *world_instance = world_res->instantiate();
  ERR_FAIL_COND_MSG(!world_instance,
                    "WorldLoader: Nie udało się utworzyć instancji świata");

  Node *terrain_node = world_instance->find_child("Terrain", true, false);
  VoxelTerrain *terrain = Object::cast_to<VoxelTerrain>(terrain_node);

  if (terrain) {
    if (is_server) {
      const String sqlite_path = save_path.path_join("terrain.sqlite");
      LOG("WorldLoader: Inicjalizacja SQLite: %s", sqlite_path);

      Ref<VoxelStreamSQLite> stream;
      stream.instantiate();
      stream->set_database_path(sqlite_path);
      terrain->set_stream(stream);
      Ref<VoxelGeneratorGraph> gen = terrain->get_generator();
      if (gen.is_valid()) {
        apply_seed_to_all_noises(gen, final_seed);
      } else {
        WARN_PRINT("WorldLoader: Terrain generator is not VoxelGeneratorGraph");
      }

    } else {
      terrain->set_stream(Ref<VoxelStreamSQLite>());
    }
  }

  _change_scene_to_instance(world_instance);
}

void WorldLoader::_change_scene_to_instance(Node *new_scene_instance) {
  SceneTree *tree = get_tree();
  Window *root = tree->get_root();
  Node *current_scene = tree->get_current_scene();

  if (current_scene) {
    current_scene->queue_free();
  }
  root->add_child(new_scene_instance);
  tree->set_current_scene(new_scene_instance);
}

static int _find_param_index(const godot::Ref<godot::VoxelGraphFunction> &fn,
                             uint32_t node_id,
                             const godot::String &param_name) {
  godot::Dictionary info =
      fn->get_node_type_info(fn->get_node_type_id(node_id));
  godot::Array params = info.get("params", godot::Array());
  for (int i = 0; i < params.size(); i++) {
    godot::Dictionary p = params[i];
    if (godot::String(p.get("name", "")) == param_name) {
      return i;
    }
  }
  return -1;
}

void WorldLoader::apply_seed_to_all_noises(
    const godot::Ref<godot::VoxelGeneratorGraph> &gen, int seed) {
  ERR_FAIL_COND_MSG(!gen.is_valid(), "GeneratorGraph is null");

  godot::Ref<godot::VoxelGraphFunction> fn = gen->get_main_function();
  ERR_FAIL_COND_MSG(!fn.is_valid(), "VoxelGraphFunction is null");

  godot::PackedInt32Array ids = fn->get_node_ids();
  for (int i = 0; i < ids.size(); i++) {
    uint32_t id = ids[i];
    const auto type = fn->get_node_type_id(id);

    const bool is_noise =
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_2D ||
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_3D ||
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_GRADIENT_2D ||
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_GRADIENT_3D ||
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_2_2D ||
        type == godot::VoxelGraphFunction::NODE_FAST_NOISE_2_3D;

    if (!is_noise) {
      continue;
    }

    int idx = _find_param_index(fn, id, "noise");
    if (idx < 0) {
      continue;
    }

    godot::Variant v = fn->get_node_param(id, idx);
    if (v.get_type() != godot::Variant::OBJECT) {
      continue;
    }

    godot::Object *obj = v;
    if (!obj) {
      continue;
    }

    if (obj->has_method("set_seed")) {
      obj->call("set_seed", seed);
    }
  }
}

void WorldLoader::_bind_methods() {
  ClassDB::bind_method(
      D_METHOD("load_game", "save_path", "is_server", "use_seed", "seed"),
      &WorldLoader::load_game);
}

}
