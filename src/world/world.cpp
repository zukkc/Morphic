#include "world.h"
#include "core/network_manager.h"
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

World::World() {
  _terrain = nullptr;
  _player_spawner = nullptr;
}
World::~World() {}

void World::_enter_tree() {}

void World::_ready() {
  setup_player_spawner();
  connect_terrain_node();
  check_player_scene_status();

  if (NetUtils::is_server(this)) {
    setup_server();
  } else {
    setup_client();
  }

  set_voxel_tool();
}

// private

void World::setup_player_spawner() {
  _player_spawner =
      Object::cast_to<MultiplayerSpawner>(get_node_or_null("PlayerSpawner"));
  if (!_player_spawner) {
    ERR("CRITICAL: Brak PlayerSpawner w World!");
    return;
  }

  _player_spawner->set_spawn_function(Callable(this, "create_player"));
}

void World::setup_server() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  NetworkManager *net_manager = NetUtils::get_net_manager(this);

  if (!net_manager) {
    ERR("CRITICAL: Brak NetworkManagera w /root!");
    return;
  }

  Error err = net_manager->connect("player_joined",
                                   Callable(this, "server_spawn_player"));
  if (err != OK) {
    ERR("Connect player_joined failed: %d", err);
  }
  net_manager->connect("player_left", Callable(this, "server_despawn_player"));

  Dictionary players = net_manager->get_player_list();
  Array ids = players.keys();

  for (int i = 0; i < ids.size(); i++) {
    int id = ids[i];
    server_spawn_player(id);
  }

  if (_terrain) {
    _terrain->set_generate_collisions(true);
  }
}

void World::setup_client() {
  if (_terrain) {
    _terrain->set_generator(Ref<VoxelGenerator>());
    _terrain->set_generate_collisions(true);
    _terrain->set_stream(Ref<VoxelStream>());
  }
}

//////// SERVER ////////////////

void World::server_spawn_player(int p_peer_id) {
  ERR_FAIL_COND(!NetUtils::is_server(this));
  if (!_player_spawner) {
    ERR("PlayerSpawner is not set");
    return;
  }

  if (!_player_scene_prefab.is_valid()) {
    ERR("Player has not been spawn. Prefab is not valid");
    return;
  }

  Node *players_root = get_node_or_null("Players");
  if (!players_root) {
    ERR("Players root not found");
    return;
  }

  String node_name = String::num(p_peer_id);
  if (players_root->has_node(node_name)) {
    LOG("Player %d already exists. Skipping spawn.", p_peer_id);
    return;
  }

  Vector3 spawn_pos = calc_spawn_position();
  Dictionary data;
  data["peer_id"] = p_peer_id;
  data["spawn_pos"] = spawn_pos;

  Node *spawned = _player_spawner->spawn(data);
  if (spawned) {
    LOG("Spawned player %d at: %s", p_peer_id, spawn_pos);
  }
}

void World::server_despawn_player(int p_peer_id) {
  ERR_FAIL_COND(!NetUtils::is_server(this));

  Node *players_root = get_node_or_null("Players");
  if (!players_root) {
    ERR("Players root not found");
    return;
  }

  VAR(NetUtils::get_net_manager(this)->get_player_list());

  Node *player_node = players_root->get_node_or_null(String::num(p_peer_id));
  if (!player_node) {
    WARN("Player node not found for peer_id %d", p_peer_id);
    return;
  }

  Player *player = cast_to<Player>(player_node);
  if (player) {
    _players.erase(player);
  }

  player_node->queue_free();
}

Node *World::create_player(const Variant &p_data) {
  if (!_player_scene_prefab.is_valid()) {
    ERR("Player scene is not valid");
    return nullptr;
  }

  Dictionary data = p_data;
  int peer_id = data.get("peer_id", 1);
  Vector3 spawn_pos = data.get("spawn_pos", Vector3());

  Node *player_instance = _player_scene_prefab->instantiate();
  Player *player = cast_to<Player>(player_instance);
  if (!player) {
    ERR("Failed casting to type of Player!");
    memdelete(player_instance);
    return nullptr;
  }

  player->set_peer_id(peer_id);
  player->set_name(String::num(peer_id));
  player->set_position(spawn_pos);
  _players.push_back(player);

  return player;
}

////////////////////////////////////

Vector3 World::calc_spawn_position() {
  if (_terrain && _vt.is_valid()) {
    Ref<VoxelGenerator> gen = _terrain->get_generator();

    if (gen.is_valid()) {
      for (int y = -1; y > -100; y--) {

        const int bs = _terrain->get_data_block_size();

        Ref<VoxelBuffer> buf;
        buf.instantiate();
        buf->create(bs, bs, bs);

        Vector3i voxel_pos(5, y, 5);
        Vector3i block_pos = _terrain->voxel_to_data_block(Vector3(voxel_pos));
        Vector3i block_origin = _terrain->data_block_to_voxel(block_pos);

        gen->generate_block(buf, block_origin, 0);

        Vector3i local = voxel_pos - block_origin;
        float v = buf->get_voxel_f(local.x, local.y, local.z,
                                   VoxelBuffer::CHANNEL_SDF);

        if (v < -0.5f) {
          return Vector3(5, y + 2, 5);
        }
      }
    }
  }

  WARN("Failed to find good place to spawn player");
  return Vector3(0, -10, 0);
}

void World::set_voxel_tool() {
  if (!_terrain) {
    ERR("Failed getting instance of voxel tool");
    return;
  }
  _vt = _terrain->get_voxel_tool();
}

// inspector getter and setter

void World::set_terrain_path(NodePath path) { _terrain_path = path; }

NodePath World::get_terrain_path() const { return _terrain_path; }

void World::connect_terrain_node() {
  if (_terrain_path.is_empty()) {
    ERR("Terrain path is not set in World");
    return;
  }

  Node *terrain_node = get_node_or_null(_terrain_path);
  _terrain = cast_to<VoxelTerrain>(terrain_node);

  if (_terrain) {
    LOG("Connected terain succesfuly");
  } else {
    ERR("Cant find Terrain node");
  }
}

void World::set_player_scene(const Ref<PackedScene> &p_player_scene) {
  _player_scene_prefab = p_player_scene;
}

Ref<PackedScene> World::get_player_scene() const {
  return _player_scene_prefab;
}

void World::check_player_scene_status() {
  if (_player_scene_prefab.is_null())
    ERR("Player scene has not been set");
}

void World::_bind_methods() {

  ClassDB::bind_method(D_METHOD("create_player", "data"),
                       &World::create_player);

  ClassDB::bind_method(D_METHOD("server_spawn_player", "p_peer_id"),

                       &World::server_spawn_player);

  ClassDB::bind_method(D_METHOD("server_despawn_player", "p_peer_id"),

                       &World::server_despawn_player);

  ClassDB::bind_method(D_METHOD("set_terrain_path", "p_path"),
                       &World::set_terrain_path);
  ClassDB::bind_method(D_METHOD("get_terrain_path"), &World::get_terrain_path);
  ADD_PROPERTY(PropertyInfo(Variant::NODE_PATH, "terrain_path",
                            PROPERTY_HINT_NODE_PATH_VALID_TYPES,
                            "VoxelTerrain"),
               "set_terrain_path", "get_terrain_path");

  ClassDB::bind_method(D_METHOD("set_player_scene", "p_player_scene"),
                       &World::set_player_scene);
  ClassDB::bind_method(D_METHOD("get_player_scene"), &World::get_player_scene);

  ADD_PROPERTY(PropertyInfo(Variant::OBJECT, "player_scene",
                            PROPERTY_HINT_RESOURCE_TYPE, "PackedScene"),
               "set_player_scene", "get_player_scene");
}

} // namespace morphic
