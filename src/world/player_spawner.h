#pragma once

#include "player/player.h"

#include "godot_cpp/classes/multiplayer_spawner.hpp"
#include "godot_cpp/classes/packed_scene.hpp"

using namespace godot;

namespace morphic {

class PlayerSpawner : public MultiplayerSpawner {
  GDCLASS(PlayerSpawner, MultiplayerSpawner)

protected:
  static void _bind_methods();

public:
  void _ready() override;

  Ref<PackedScene> get_player_scene() const;
  void set_player_scene(const Ref<PackedScene> &p_scene);

private:
  Node *_players_root = nullptr;
  Ref<PackedScene> _player_scene_prefab;

  // server

  void server_bind_spawner_to_network();
  void server_spawn_player(int p_peer_id);
  void server_despawn_player(int p_peer_id);

  Node *create_player(const Variant &p_data);
  Vector3 calc_spawn_position();
  Player *find_player(int p_peer_id);
};

} // namespace morphic
