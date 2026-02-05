#pragma once

#include "godot_cpp/classes/voxel_generator_graph.hpp"
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
  void _enter_tree() override;
  void _ready() override;

  // should be called by WorldLoader
  void setup_server(Dictionary p_save_info);
  void setup_client(Dictionary p_save_info);

private:
  NodePath _terrain_path;
  VoxelTerrain *_terrain = nullptr;
  Ref<VoxelTool> _vt;

  void set_voxel_tool();

  // inspector getters and setters

  NodePath get_terrain_path() const;
  void set_terrain_path(const NodePath p_path);
  void connect_terrain_node();
  void apply_seed_to_all_graph_noises(Ref<VoxelGeneratorGraph> generator,
                                      int global_seed);
  void _on_mesh_block_entered(Vector3i p_pos);
};

} // namespace morphic
