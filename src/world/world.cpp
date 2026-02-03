#include "world.h"
#include "utils/bind_methods.h"
#include "utils/debug_utils.h"
#include "utils/network_utils.h"

#include <godot_cpp/classes/display_server.hpp>
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/voxel_generator.hpp>
#include <godot_cpp/classes/voxel_stream.hpp>

using namespace godot;

namespace morphic {

void World::_ready() {
  connect_terrain_node();

  if (NetUtils::is_server(this)) {
    setup_server();
    set_voxel_tool();
  } else {
    setup_client();
  }
}

// private

void World::setup_server() {
  ERR_FAIL_COND(!NetUtils::is_server(this));

  if (_terrain) {
    _terrain->set_generate_collisions(true);
  }
}

void World::setup_client() {
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

  BIND_PROPERTY_HINT(World, Variant::NODE_PATH, "terrain_path", terrain_path,
                     PROPERTY_HINT_NODE_PATH_VALID_TYPES);
}

} // namespace morphic
