#pragma once

#include <godot_cpp/classes/multiplayer_spawner.hpp>
#include <godot_cpp/classes/node3d.hpp>
#include <godot_cpp/classes/packed_scene.hpp>
#include <godot_cpp/classes/voxel_terrain.hpp>
#include <godot_cpp/classes/voxel_tool.hpp>

using namespace godot;

namespace morphic {

class World : public Node3D {
  GDCLASS(World, Node3D)

protected:
  static void _bind_methods();

public:
  World();
  ~World();

  void _enter_tree() override;
  void _ready() override;

  // static
  static Vector3 compute_normal_one_sided(const Ref<VoxelBuffer> &buf,
                                          const Vector3i &p, int bs);

private:
  MultiplayerSpawner *_player_spawner;

  NodePath _terrain_path;
  VoxelTerrain *_terrain;
  Ref<PackedScene> _player_scene_prefab;
  Ref<VoxelTool> _vt;

  void setup_server();
  void setup_client();
  void setup_player_spawner();

  Vector3 calc_spawn_position();
  Node *create_player(const Variant &p_data);
  void set_voxel_tool();

  // server

  void server_spawn_player(int p_peer_id);
  void server_despawn_player(int p_peer_id);

  // inspector getters and setters

  NodePath get_terrain_path() const;
  void set_terrain_path(const NodePath p_path);
  Ref<PackedScene> get_player_scene() const;
  void set_player_scene(const Ref<PackedScene> &p_scene);

  void connect_terrain_node();
  void check_player_scene_status();
};

} // namespace morphic
