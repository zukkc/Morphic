#include "world_loader.h"

#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/class_db.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace morphic {

void WorldLoader::_begin_threaded_load(const String &path) {
  _path = path;
  _loading = true;

  // Kick off threaded load
  Error err = ResourceLoader::get_singleton()->load_threaded_request(_path);
  ERR_FAIL_COND_MSG(err != OK, "WorldLoader: load_threaded_request failed");
}

void WorldLoader::load_world_from_save_async(const String &world_scene_path,
                                             int mode,
                                             const Dictionary &save_info) {
  ERR_FAIL_COND_MSG(_loading, "WorldLoader: already loading");
  ERR_FAIL_COND_MSG(world_scene_path.is_empty(),
                    "WorldLoader: world_scene_path empty");

  _mode = mode;
  _payload = save_info;
  _begin_threaded_load(world_scene_path);

  emit_signal("loading_started");
}

void WorldLoader::load_world_as_client_async(const String &world_scene_path) {
  ERR_FAIL_COND_MSG(_loading, "WorldLoader: already loading");
  ERR_FAIL_COND_MSG(world_scene_path.is_empty(),
                    "WorldLoader: world_scene_path empty");

  _mode = MODE_CLIENT;
  _begin_threaded_load(world_scene_path);

  emit_signal("loading_started");
}

void WorldLoader::_process(double) {
  if (!_loading)
    return;

  Array progress_arr;
  ResourceLoader::ThreadLoadStatus st =
      ResourceLoader::get_singleton()->load_threaded_get_status(_path,
                                                                progress_arr);

  float p = 0.0f;
  if (progress_arr.size() > 0) {
    p = (float)progress_arr[0];
  }
  emit_signal("loading_progress", p);

  if (st == ResourceLoader::THREAD_LOAD_FAILED) {
    _loading = false;
    emit_signal("loading_failed",
                String("WorldLoader: threaded load failed for: ") + _path);
    return;
  }

  if (st != ResourceLoader::THREAD_LOAD_LOADED) {
    return; // still loading
  }

  Ref<Resource> res = ResourceLoader::get_singleton()->load_threaded_get(_path);
  _loading = false;

  Ref<PackedScene> packed = res;
  if (packed.is_null()) {
    emit_signal("loading_failed",
                "WorldLoader: loaded resource is not PackedScene");
    return;
  }

  _finish_and_switch(packed);
}

void WorldLoader::_finish_and_switch(const Ref<PackedScene> &packed) {
  Node *instance = packed->instantiate();
  if (!instance) {
    emit_signal("loading_failed",
                "WorldLoader: failed to instantiate world scene");
    return;
  }

  // Switch scene
  _replace_current_scene(instance);

  // Configure root AFTER switching so world root is already inside SceneTree.
  // setup_server/setup_client rely on get_tree()/multiplayer context.
  _configure_world_root(instance);

  emit_signal("loading_finished");
}

void WorldLoader::_configure_world_root(Node *world_root) {
  if (!world_root)
    return;

  // Contract-based config calls; no brittle node searching here.
  switch (_mode) {
  case MODE_SINGLEPLAYER:
    // if (world_root->has_method("configure_singleplayer")) {
    //   world_root->call("configure_singleplayer", _payload);
    // } else if (world_root->has_method("configure_server")) {
    //   // fallback
    //   world_root->call("configure_server", _payload);
    // } else {
    //   WARN_PRINT("WorldLoader: world root missing "
    //              "configure_singleplayer/configure_server");
    // }
    break;

  case MODE_HOST:
  case MODE_DEDICATED:
    if (world_root->has_method("setup_server")) {
      world_root->call("setup_server", _payload);
    } else {
      WARN_PRINT("WorldLoader: world root missing setup_server");
    }
    break;

  case MODE_CLIENT:
    if (world_root->has_method("setup_client")) {
      world_root->call("setup_client", _payload);
    } else {
      WARN_PRINT("WorldLoader: world root missing setup_client");
    }
    break;

  default:
    WARN_PRINT("WorldLoader: unknown mode");
    break;
  }
}

void WorldLoader::_replace_current_scene(Node *new_scene_instance) {
  SceneTree *tree = get_tree();
  ERR_FAIL_COND(!tree);

  Window *root = tree->get_root();
  ERR_FAIL_COND(!root);

  Node *current_scene = tree->get_current_scene();
  if (current_scene) {
    current_scene->queue_free();
  }

  root->add_child(new_scene_instance);
  tree->set_current_scene(new_scene_instance);
}

void WorldLoader::_bind_methods() {
  BIND_ENUM_CONSTANT(MODE_SINGLEPLAYER);
  BIND_ENUM_CONSTANT(MODE_HOST);
  BIND_ENUM_CONSTANT(MODE_DEDICATED);
  BIND_ENUM_CONSTANT(MODE_CLIENT);

  ClassDB::bind_method(D_METHOD("load_world_from_save_async",
                                "world_scene_path", "mode", "save_info"),
                       &WorldLoader::load_world_from_save_async);
  ClassDB::bind_method(
      D_METHOD("load_world_as_client_async", "world_scene_path"),
      &WorldLoader::load_world_as_client_async);

  ADD_SIGNAL(MethodInfo("loading_started"));
  ADD_SIGNAL(
      MethodInfo("loading_progress", PropertyInfo(Variant::FLOAT, "progress")));
  ADD_SIGNAL(
      MethodInfo("loading_failed", PropertyInfo(Variant::STRING, "error")));
  ADD_SIGNAL(MethodInfo("loading_cancelled"));
  ADD_SIGNAL(MethodInfo("loading_finished"));
}

} // namespace morphic
