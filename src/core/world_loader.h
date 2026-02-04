#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/ref.hpp>

namespace godot {
class VoxelGeneratorGraph;
}

using namespace godot;

namespace morphic {

class WorldLoader : public Node {
  GDCLASS(WorldLoader, Node)
protected:
  static void _bind_methods();

public:
  WorldLoader();
  ~WorldLoader();

  void load_game(const String &save_path, bool is_server,
                 bool use_seed = false, int seed = 0);
  void _change_scene_to_instance(Node *new_scene_instance);

private:
  String world_scene_path = "res://world/world.tscn";
  bool ensure_save_directory(const String &save_dir_path);
  bool prepare_world_config(const String &save_dir_path, bool use_seed,
                            int seed, int &out_seed);
  void apply_seed_to_all_noises(const godot::Ref<godot::VoxelGeneratorGraph> &gen,
                                int seed);
};

} // namespace morphic
