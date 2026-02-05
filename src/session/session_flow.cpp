#include "session_flow.h"

#include "core/network_manager.h"
#include "saves/save_manager.h"
#include "utils/debug_utils.h"
#include "world/world_loader.h"

using namespace godot;

namespace morphic {

bool SessionFlow::configure(NetworkManager *net_manager,
                            WorldLoader *world_loader,
                            SaveManager *save_manager) {
  _net_manager = net_manager;
  _world_loader = world_loader;
  _save_manager = save_manager;

  if (!dependencies_ready()) {
    return false;
  }

  _state = IDLE;
  _epoch = 0;
  _loading_epoch = 0;
  _connecting_epoch = 0;
  _pending_join_address = "";
  _pending_join_port = 0;
  _pending_expected_world_id = "";
  return true;
}

bool SessionFlow::request_host(const String &world_scene_path, int host_port,
                               const String &saves_path,
                               const String &save_name, int seed) {
  if (!dependencies_ready()) {
    return false;
  }

  if (!can_start_new_flow()) {
    ERR_PRINT("SessionFlow: request_host rejected. Flow already in progress.");
    return false;
  }

  ERR_FAIL_COND_V_MSG(world_scene_path.is_empty(), false,
                      "SessionFlow: world_scene_path is empty");

  ++_epoch;
  _loading_epoch = 0;
  _connecting_epoch = 0;

  if (!transition(HOST_STARTING, "Host start requested")) {
    return false;
  }

  if (!_net_manager->start_host(host_port)) {
    return fail("Failed starting host");
  }

  const String save_path = build_save_path(saves_path, save_name);
  if (save_path.is_empty()) {
    return fail("Save path is empty. Set saves_path/save_name");
  }

  Dictionary save = _save_manager->create_new(save_path, seed);
  if (save.is_empty()) {
    // Existing save is a normal case for host flow; fallback to load_existing.
    save = _save_manager->load_existing(save_path);
  }
  if (save.is_empty()) {
    return fail("Failed creating/loading save data");
  }

  const String world_id = String(save.get("save_dir", ""));
  const int world_seed = (int)save.get("seed", 0);
  _net_manager->configure_server_handshake_context(world_id, world_seed);

  if (!transition(WORLD_LOADING_HOST, "Loading host world")) {
    return false;
  }
  _loading_epoch = _epoch;
  _world_loader->load_world_from_save_async(world_scene_path,
                                            WorldLoader::MODE_HOST, save);
  return true;
}

bool SessionFlow::request_join(const String &world_scene_path,
                               const String &join_address, int join_port,
                               const String &expected_world_id) {
  if (!dependencies_ready()) {
    return false;
  }

  if (!can_start_new_flow()) {
    ERR_PRINT("SessionFlow: request_join rejected. Flow already in progress.");
    return false;
  }

  ERR_FAIL_COND_V_MSG(world_scene_path.is_empty(), false,
                      "SessionFlow: world_scene_path is empty");
  ERR_FAIL_COND_V_MSG(join_address.is_empty(), false,
                      "SessionFlow: join_address is empty");

  ++_epoch;
  _loading_epoch = _epoch;
  _connecting_epoch = 0;
  _pending_join_address = join_address;
  _pending_join_port = join_port;
  _pending_expected_world_id = expected_world_id;

  if (!transition(WORLD_LOADING_CLIENT, "Loading client world")) {
    return false;
  }

  _world_loader->load_world_as_client_async(world_scene_path);
  return true;
}

void SessionFlow::request_cancel() {
  ++_epoch;
  _loading_epoch = 0;
  _connecting_epoch = 0;
  _pending_join_address = "";
  _pending_join_port = 0;
  _pending_expected_world_id = "";
  transition(IDLE, "Request cancelled");
}

void SessionFlow::on_world_loading_finished() {
  if (_state == WORLD_LOADING_CLIENT && _loading_epoch == _epoch) {
    if (!transition(CLIENT_CONNECTING, "Client world loaded")) {
      return;
    }
    _connecting_epoch = _epoch;
    _net_manager->configure_client_handshake_context(_pending_expected_world_id,
                                                     0);
    if (!_net_manager->start_client(_pending_join_address, _pending_join_port)) {
      fail("Failed connecting to server");
    }
    return;
  }

  if (_state == WORLD_LOADING_HOST && _loading_epoch == _epoch) {
    transition(IN_GAME, "Host world loaded");
    return;
  }
}

void SessionFlow::on_world_loading_failed(const String &error) {
  if ((_state == WORLD_LOADING_CLIENT || _state == WORLD_LOADING_HOST) &&
      _loading_epoch == _epoch) {
    fail(String("World loading failed: ") + error);
  }
}

void SessionFlow::on_connection_success() {
  if (_state == CLIENT_CONNECTING && _connecting_epoch == _epoch) {
    transition(IN_GAME, "Client connected");
  }
}

void SessionFlow::on_connection_failed() {
  if (_state == CLIENT_CONNECTING && _connecting_epoch == _epoch) {
    fail("Connection failed");
  }
}

SessionFlow::State SessionFlow::get_state() const { return _state; }

const char *SessionFlow::state_to_cstr(State state) {
  switch (state) {
  case IDLE:
    return "IDLE";
  case HOST_STARTING:
    return "HOST_STARTING";
  case WORLD_LOADING_HOST:
    return "WORLD_LOADING_HOST";
  case WORLD_LOADING_CLIENT:
    return "WORLD_LOADING_CLIENT";
  case CLIENT_CONNECTING:
    return "CLIENT_CONNECTING";
  case IN_GAME:
    return "IN_GAME";
  case ERROR_STATE:
    return "ERROR_STATE";
  default:
    return "UNKNOWN";
  }
}

bool SessionFlow::dependencies_ready() const {
  if (!_net_manager || !_world_loader || !_save_manager) {
    ERR_PRINT("SessionFlow: dependencies are not ready");
    return false;
  }
  return true;
}

bool SessionFlow::can_start_new_flow() const {
  return _state == IDLE || _state == ERROR_STATE;
}

String SessionFlow::build_save_path(const String &saves_path,
                                    const String &save_name) const {
  if (!saves_path.is_empty() && !save_name.is_empty()) {
    return saves_path.path_join(save_name);
  }
  if (saves_path.is_empty() && !save_name.is_empty()) {
    return save_name;
  }
  return saves_path;
}

bool SessionFlow::transition(State next_state, const String &reason) {
  if (_state == next_state) {
    return true;
  }

  LOG("SessionFlow: %s -> %s (%s) [epoch=%d]", state_to_cstr(_state),
      state_to_cstr(next_state), reason, (int)_epoch);
  _state = next_state;
  return true;
}

bool SessionFlow::fail(const String &reason) {
  ERR_PRINT(String("SessionFlow: ") + reason);
  return transition(ERROR_STATE, reason);
}

} // namespace morphic
