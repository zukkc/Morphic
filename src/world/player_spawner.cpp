#include "player_spawner.h"
#include "utils/bind_methods.h"
#include "utils/network_utils.h"

#include "godot_cpp/classes/engine.hpp"

using namespace godot;

namespace morphic {

void PlayerSpawner::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  set_spawn_function(Callable(this, "create_player"));
  _players_root = get_node_or_null(get_spawn_path());
  ERR_FAIL_COND_MSG(
      !_players_root,
      "Cant find Players node in scene. Check if spawn path is set");

  if (!NetUtils::is_server(this))
    return;

  server_bind_spawner_to_network();
}

//////// SERVER ////////////////

void PlayerSpawner::server_bind_spawner_to_network() {
  ERR_FAIL_COND_MSG(!NetUtils::is_server(this),
                    "Client tried bind PlayerSpawner to network");

  NetworkManager *net_manager = NetUtils::get_net_manager(this);

  ERR_FAIL_COND_MSG(!net_manager, "CRITICAL: Brak NetworkManagera w /root!");

  net_manager->connect("player_joined", Callable(this, "server_spawn_player"));
  net_manager->connect("player_left", Callable(this, "server_despawn_player"));

  Dictionary players = net_manager->get_player_list();
  Array ids = players.keys();

  for (int i = 0; i < ids.size(); i++) {
    int id = ids[i];
    server_spawn_player(id);
  }
}

void PlayerSpawner::server_spawn_player(int p_peer_id) {
  ERR_FAIL_COND(!NetUtils::is_server(this));
  ERR_FAIL_COND_MSG(!_player_scene_prefab.is_valid(),
                    "Player has not been spawn. Prefab is not valid");
  ERR_FAIL_COND_MSG(!_players_root, "Players root not found");

  String node_name = String::num(p_peer_id);
  if (find_player(p_peer_id)) {
    LOG("Player %d already exists. Skipping spawn.", p_peer_id);
    return;
  }

  Vector3 spawn_pos = calc_spawn_position();
  Dictionary data;
  data["peer_id"] = p_peer_id;
  data["spawn_pos"] = spawn_pos;

  Node *spawned = spawn(data);
  if (spawned) {
    LOG("Spawned player %d at: %s", p_peer_id, spawn_pos);
  }
}

void PlayerSpawner::server_despawn_player(int p_peer_id) {
  ERR_FAIL_COND(!NetUtils::is_server(this));
  ERR_FAIL_COND_MSG(!_players_root, "Players root not found");

  Player *existing_player = find_player(p_peer_id);

  if (existing_player) {
    existing_player->queue_free();
    LOG("Player %d removed", p_peer_id);
    return;
  }

  LOG("Player %d not found. Cant remove", p_peer_id);
}

////////////////////////////

Node *PlayerSpawner::create_player(const Variant &p_data) {
  ERR_FAIL_COND_V_MSG(!_player_scene_prefab.is_valid(), nullptr,
                      "Player scene is not valid");

  Dictionary data = p_data;
  int peer_id = data.get("peer_id", 1);
  Vector3 spawn_pos = data.get("spawn_pos", Vector3());

  Node *player_instance = _player_scene_prefab->instantiate();
  Player *player = cast_to<Player>(player_instance);
  if (!player) {
    ERR_PRINT_ONCE("Failed casting to type of Player!");
    memdelete(player_instance);
    return nullptr;
  }

  player->set_multiplayer_authority(peer_id);
  player->set_peer_id(peer_id);
  player->set_name(String::num(peer_id));
  player->set_position(spawn_pos);

  return player;
}

Vector3 PlayerSpawner::calc_spawn_position() { return Vector3(0, -2, 0); }

Player *PlayerSpawner::find_player(int p_peer_id) {
  ERR_FAIL_COND_V_MSG(!_players_root, nullptr, "Players root not found");

  for (int i = 0; i < _players_root->get_child_count(); i++) {
    Node *child = _players_root->get_child(i);
    Player *player = Object::cast_to<Player>(child);
    if (player && player->get_peer_id() == p_peer_id) {
      return player;
    }
  }

  return nullptr;
}

////////////////////

void PlayerSpawner::set_player_scene(const Ref<PackedScene> &p_player_scene) {
  _player_scene_prefab = p_player_scene;
}

Ref<PackedScene> PlayerSpawner::get_player_scene() const {
  return _player_scene_prefab;
}

void PlayerSpawner::_bind_methods() {
  ClassDB::bind_method(D_METHOD("create_player", "data"),
                       &PlayerSpawner::create_player);

  ClassDB::bind_method(D_METHOD("server_spawn_player", "p_peer_id"),
                       &PlayerSpawner::server_spawn_player);

  ClassDB::bind_method(D_METHOD("server_despawn_player", "p_peer_id"),
                       &PlayerSpawner::server_despawn_player);

  BIND_PROPERTY_HINT(PlayerSpawner, Variant::OBJECT, "player_scene",
                     player_scene, PROPERTY_HINT_RESOURCE_TYPE);
}

} // namespace morphic
