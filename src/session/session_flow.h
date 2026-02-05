#pragma once

#include <cstdint>

#include <godot_cpp/variant/string.hpp>

namespace morphic {

class NetworkManager;
class SaveManager;
class WorldLoader;

class SessionFlow {
public:
  enum State {
    IDLE = 0,
    HOST_STARTING = 1,
    WORLD_LOADING_HOST = 2,
    WORLD_LOADING_CLIENT = 3,
    CLIENT_CONNECTING = 4,
    IN_GAME = 5,
    ERROR_STATE = 6
  };

  bool configure(NetworkManager *net_manager, WorldLoader *world_loader,
                 SaveManager *save_manager);

  bool request_host(const godot::String &world_scene_path, int host_port,
                    const godot::String &saves_path,
                    const godot::String &save_name, int seed);
  bool request_join(const godot::String &world_scene_path,
                    const godot::String &join_address, int join_port,
                    const godot::String &expected_world_id);
  void request_cancel();

  void on_world_loading_finished();
  void on_world_loading_failed(const godot::String &error);
  void on_connection_success();
  void on_connection_failed();

  State get_state() const;
  static const char *state_to_cstr(State state);

private:
  NetworkManager *_net_manager = nullptr;
  WorldLoader *_world_loader = nullptr;
  SaveManager *_save_manager = nullptr;

  State _state = IDLE;
  uint64_t _epoch = 0;
  uint64_t _loading_epoch = 0;
  uint64_t _connecting_epoch = 0;

  godot::String _pending_join_address;
  int _pending_join_port = 0;
  godot::String _pending_expected_world_id;

  bool dependencies_ready() const;
  bool can_start_new_flow() const;
  godot::String build_save_path(const godot::String &saves_path,
                                const godot::String &save_name) const;
  bool transition(State next_state, const godot::String &reason);
  bool fail(const godot::String &reason);
};

} // namespace morphic
