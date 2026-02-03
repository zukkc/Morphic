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
  void _ready() override;

  // static
  static Vector3 compute_normal_one_sided(const Ref<VoxelBuffer> &buf,
                                          const Vector3i &p, int bs);

private:
  NodePath _terrain_path;
  VoxelTerrain *_terrain = nullptr;
  Ref<VoxelTool> _vt;

  void setup_server();
  void setup_client();

  void set_voxel_tool();

  // inspector getters and setters

  NodePath get_terrain_path() const;
  void set_terrain_path(const NodePath p_path);
  void connect_terrain_node();
};

} // namespace morphic
