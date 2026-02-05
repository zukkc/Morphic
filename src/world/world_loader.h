#pragma once

#include "godot_cpp/classes/packed_scene.hpp"
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace morphic {

class WorldLoader : public Node {
  GDCLASS(WorldLoader, Node);

public:
  enum WorldMode {
    MODE_SINGLEPLAYER = 0,
    MODE_HOST = 1,
    MODE_DEDICATED = 2,
    MODE_CLIENT = 3
  };

  WorldLoader() = default;
  ~WorldLoader() = default;

  void _process(double delta) override;

  // Start world from an existing/new save (server-ish modes)
  void load_world_from_save_async(const String &world_scene_path, int mode,
                                  const Dictionary &save_info);

  // Start client join (no save)
  void load_world_as_client_async(const String &world_scene_path);

protected:
  static void _bind_methods();

private:
  bool _loading = false;
  String _path;
  int _mode = MODE_SINGLEPLAYER;
  Dictionary _payload; // save_info

  void _begin_threaded_load(const String &path);

  void _finish_and_switch(const Ref<PackedScene> &packed);

  // Calls configure_* on world root
  void _configure_world_root(Node *world_root);

  // Scene switch helper (safe-ish)
  void _replace_current_scene(Node *new_scene_instance);
};

} // namespace morphic

VARIANT_ENUM_CAST(morphic::WorldLoader::WorldMode);
