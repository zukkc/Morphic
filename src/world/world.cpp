#include "world.h"
#include "utils/bind_methods.h"
#include "utils/debug_utils.h"
#include "utils/network_utils.h"

#include "godot_cpp/classes/voxel_graph_function.hpp"
#include "godot_cpp/classes/voxel_stream_sq_lite.hpp"
#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/voxel_stream.hpp>

using namespace godot;

namespace morphic {

void World::_enter_tree() {
  connect_terrain_node();
  _terrain->connect("mesh_block_entered",
                    Callable(this, "_on_mesh_block_entered"));
}

void World::_ready() {
  if (NetUtils::is_server(this)) {
    set_voxel_tool();
  }
}

void World::setup_server(Dictionary p_save_info) {
  ERR_FAIL_COND(!NetUtils::is_server(this));
  ERR_FAIL_COND_MSG(!_terrain, "Cant setup server. _terrain is nullptr");
  ERR_FAIL_COND_MSG(!_terrain, "Terrain Doesnt exist. Cant setup server");

  apply_seed_to_all_graph_noises(_terrain->get_generator(),
                                 p_save_info["seed"]);

  _terrain->set_generate_collisions(true);
  Ref<VoxelStreamSQLite> stream = memnew(VoxelStreamSQLite);
  stream->set_database_path(p_save_info["terrain_db_path"]);
  _terrain->set_stream(stream);
}

void World::setup_client(Dictionary p_save_info) {
  ERR_FAIL_COND_MSG(!_terrain, "Cant setup client. _terrain is nullptr");

  _terrain->set_generator(Ref<VoxelGenerator>());
  _terrain->set_generate_collisions(true);
  _terrain->set_stream(Ref<VoxelStream>());
}

////////////////////////////////////

void World::set_voxel_tool() {
  ERR_FAIL_COND_MSG(
      !_terrain, "Failed getting instance of voxel tool. _terrain is nullptr");
  _vt = _terrain->get_voxel_tool();
}

void World::_on_mesh_block_entered(Vector3i p_pos) {
  // TODO: wprowadzic system odmrazania gracza gdy jego chunk jest gotowy
  // ma to naprawiac blad wypadania gracza pod ziemie przy spawnie
}

void World::apply_seed_to_all_graph_noises(Ref<VoxelGeneratorGraph> p_gen,
                                           int p_global_seed) {

  ERR_FAIL_COND_MSG(p_gen.is_null(), "Error: Generator is null");

  Ref<VoxelGraphFunction> graph_func = p_gen->get_main_function();
  if (graph_func.is_null()) {
    UtilityFunctions::printerr("Error: Graph function is null!");
    return;
  }

  PackedInt32Array node_ids = graph_func->get_node_ids();
  int modified_count = 0;

  for (int i = 0; i < node_ids.size(); i++) {
    int node_id = node_ids[i];

    int type_id = (int)graph_func->get_node_type_id(node_id);
    Dictionary type_info = graph_func->get_node_type_info(type_id);

    String type_name = "";
    if (type_info.has("name"))
      type_name = String(type_info["name"]);

    if (type_name.contains("FastNoise")) {
      Variant param = graph_func->get_node_param(node_id, 0);

      if (param.get_type() == Variant::OBJECT) {
        Object *obj = param;

        if (obj) {
          int new_seed = p_global_seed + node_id * 13;
          obj->set("seed", new_seed);

          graph_func->set_node_param(node_id, 0, param);

          modified_count++;
        }
      }
    }
  }

  if (modified_count > 0) {
    p_gen->compile();
    LOG("Graph recompiled. Updated seeds for %d noise nodes.", modified_count);
  } else {
    WARN_PRINT("Warning: No noise nodes found in the generator graph.");
  }
}

// inspector getter and setter

void World::set_terrain_path(NodePath path) { _terrain_path = path; }

NodePath World::get_terrain_path() const { return _terrain_path; }

void World::connect_terrain_node() {
  ERR_FAIL_COND_MSG(_terrain_path.is_empty(),
                    "Terrain path is not set in World");

  Node *terrain_node = get_node_or_null(_terrain_path);
  _terrain = cast_to<VoxelTerrain>(terrain_node);

  ERR_FAIL_COND_MSG(!_terrain, "Cant find Terrain node");
}

void World::_bind_methods() {
  ClassDB::bind_method(D_METHOD("setup_server", "save_info"),
                       &World::setup_server);
  ClassDB::bind_method(D_METHOD("setup_client", "save_info"),
                       &World::setup_client);
  ClassDB::bind_method(D_METHOD("_on_mesh_block_entered", "p_pos"),
                       &World::_on_mesh_block_entered);

  BIND_PROPERTY_HINT(World, Variant::NODE_PATH, "terrain_path", terrain_path,
                     PROPERTY_HINT_NODE_PATH_VALID_TYPES);
}

} // namespace morphic
