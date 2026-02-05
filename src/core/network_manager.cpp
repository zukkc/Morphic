#include "core/network_manager.h"

#include "utils/debug_utils.h"
#include "utils/network_utils.h"

#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>
#include <godot_cpp/classes/multiplayer_peer.hpp>
#include <godot_cpp/classes/time.hpp>
#include <vector>

using namespace godot;

namespace morphic {

void NetworkManager::_ready() {
  Ref<MultiplayerAPI> mp = get_tree()->get_multiplayer();
  mp->connect("peer_connected", Callable(this, "_on_peer_connected"));
  mp->connect("peer_disconnected", Callable(this, "_on_peer_disconnected"));
  mp->connect("connected_to_server", Callable(this, "_on_connected_to_server"));
  mp->connect("connection_failed", Callable(this, "_on_connection_failed"));
  mp->connect("server_disconnected", Callable(this, "_on_server_disconnected"));

  Dictionary rpc_any_peer;
  rpc_any_peer["rpc_mode"] = MultiplayerAPI::RPC_MODE_ANY_PEER;
  rpc_config("_rpc_client_hello", rpc_any_peer);
  rpc_config("_rpc_client_ready", rpc_any_peer);

  Dictionary rpc_authority;
  rpc_authority["rpc_mode"] = MultiplayerAPI::RPC_MODE_AUTHORITY;
  rpc_config("_rpc_server_hello_ack", rpc_authority);
  rpc_config("_rpc_server_ready_ack", rpc_authority);

  set_process(true);
}

void NetworkManager::_process(double delta) {
  const float d = static_cast<float>(delta);

  if (NetUtils::is_server(this)) {
    std::vector<int> timed_out_peers;
    for (auto &entry : _server_handshakes) {
      entry.second.timeout_s -= d;
      if (entry.second.timeout_s <= 0.0f) {
        timed_out_peers.push_back(entry.first);
      }
    }

    for (int peer_id : timed_out_peers) {
      _disconnect_peer(peer_id, "Handshake timeout");
    }
  }

  if (_client_handshake_stage != ClientHandshakeStage::NONE) {
    _client_handshake_timeout_left -= d;
    if (_client_handshake_timeout_left <= 0.0f) {
      _close_client_connection("Client handshake timeout");
    }
  }
}

bool NetworkManager::start_host(int port) {
  connected_players.clear();
  ready_players.clear();
  _server_handshakes.clear();
  _reset_client_handshake_state();

  Ref<ENetMultiplayerPeer> peer;
  peer.instantiate();
  Error err = peer->create_server(port);
  if (err != Error::OK) {
    ERR_PRINT(DebugUtils::format_log(
        "Error: failed creating the server. status: %d", err));
    return false;
  }

  NetUtils::get_mp(this)->set_multiplayer_peer(peer);
  connected_players[1] = "Host";
  emit_signal("player_joined", 1);

  if (!_server_world_id.is_empty()) {
    _mark_peer_ready(1);
  }

  LOG("Successfuly created the server");
  return true;
}

bool NetworkManager::start_client(const String &p_address, int p_port) {
  connected_players.clear();
  ready_players.clear();
  _server_handshakes.clear();
  _reset_client_handshake_state();

  Ref<ENetMultiplayerPeer> peer;
  peer.instantiate();
  Error err = peer->create_client(p_address, p_port);
  if (err != Error::OK) {
    ERR_PRINT(DebugUtils::format_log(
        "Error: failed joining to the server. status: %d", err));
    return false;
  }

  NetUtils::get_mp(this)->set_multiplayer_peer(peer);
  LOG("Successfuly joined the server");
  return true;
}

void NetworkManager::configure_server_handshake_context(const String &world_id,
                                                        int seed) {
  _server_world_id = world_id;
  _server_seed = seed;

  if (NetUtils::is_server(this) && !_server_world_id.is_empty()) {
    if (!connected_players.has(1)) {
      connected_players[1] = "Host";
      emit_signal("player_joined", 1);
    }
    _mark_peer_ready(1);
  }
}

void NetworkManager::configure_client_handshake_context(
    const String &requested_world_id, int expected_seed) {
  _client_requested_world_id = requested_world_id;
  _client_expected_seed = expected_seed;
}

void NetworkManager::_start_client_handshake() {
  ERR_FAIL_COND_MSG(NetUtils::is_server(this),
                    "NetworkManager: server tried start client handshake");

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(this);
  ERR_FAIL_COND_MSG(mp.is_null(), "NetworkManager: MultiplayerAPI is null");
  ERR_FAIL_COND_MSG(!mp->has_multiplayer_peer(),
                    "NetworkManager: multiplayer peer is not set");

  const int my_id = mp->get_unique_id();
  _client_nonce = String::num_int64(Time::get_singleton()->get_ticks_usec()) +
                  "-" + String::num_int64(my_id);
  _client_session_nonce = "";
  _client_handshake_stage = ClientHandshakeStage::WAIT_HELLO_ACK;
  _client_handshake_timeout_left = k_client_handshake_timeout_s;

  const Error err = rpc_id(1, "_rpc_client_hello", k_protocol_version,
                           _client_build_hash, _client_requested_world_id,
                           _client_nonce);
  if (err != OK) {
    _close_client_connection(
        DebugUtils::format_log("Failed sending client hello. status: %d", err));
  }
}

void NetworkManager::_rpc_client_hello(int protocol_version,
                                       const String &client_build_hash,
                                       const String &requested_world_id,
                                       const String &client_nonce) {
  ERR_FAIL_COND_MSG(!NetUtils::is_server(this),
                    "NetworkManager: client handled client hello RPC");

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(this);
  ERR_FAIL_COND_MSG(mp.is_null(), "NetworkManager: MultiplayerAPI is null");

  const int sender_id = mp->get_remote_sender_id();
  ERR_FAIL_COND_MSG(sender_id <= 0, "NetworkManager: invalid hello sender");

  ServerHandshakePeerState &state = _server_handshakes[sender_id];
  if (!state.hello_received) {
    state.timeout_s = k_server_hello_timeout_s;
  }

  bool accepted = true;
  String reject_reason;
  if (protocol_version != k_protocol_version) {
    accepted = false;
    reject_reason = "Protocol version mismatch";
  } else if (_server_world_id.is_empty()) {
    accepted = false;
    reject_reason = "Server world context not set";
  } else if (!requested_world_id.is_empty() &&
             requested_world_id != _server_world_id) {
    accepted = false;
    reject_reason = "Requested world id mismatch";
  }

  String session_nonce;
  if (accepted) {
    state.hello_received = true;
    state.ready_received = false;
    state.protocol_version = protocol_version;
    state.client_build_hash = client_build_hash;
    state.requested_world_id = requested_world_id;
    state.client_nonce = client_nonce;
    state.session_nonce = _make_server_session_nonce(sender_id);
    state.timeout_s = k_server_ready_timeout_s;
    session_nonce = state.session_nonce;
  }

  const Error ack_err = rpc_id(sender_id, "_rpc_server_hello_ack", accepted,
                               reject_reason, k_protocol_version,
                               _server_world_id, _server_seed, client_nonce,
                               session_nonce);
  if (ack_err != OK) {
    _disconnect_peer(sender_id, DebugUtils::format_log(
                                    "Failed sending hello ack. status: %d",
                                    ack_err));
    return;
  }

  if (!accepted) {
    _disconnect_peer(sender_id, String("Handshake rejected: ") + reject_reason);
  }
}

void NetworkManager::_rpc_server_hello_ack(bool accepted,
                                           const String &reject_reason,
                                           int protocol_version,
                                           const String &authoritative_world_id,
                                           int authoritative_seed,
                                           const String &client_nonce,
                                           const String &session_nonce) {
  ERR_FAIL_COND_MSG(NetUtils::is_server(this),
                    "NetworkManager: server handled server hello ack RPC");

  if (_client_handshake_stage != ClientHandshakeStage::WAIT_HELLO_ACK) {
    return;
  }

  if (client_nonce != _client_nonce) {
    _close_client_connection("Handshake nonce mismatch on hello ack");
    return;
  }

  if (!accepted) {
    _close_client_connection(String("Server rejected handshake: ") +
                             reject_reason);
    return;
  }

  if (protocol_version != k_protocol_version) {
    _close_client_connection("Server protocol version mismatch");
    return;
  }

  if (!_client_requested_world_id.is_empty() &&
      authoritative_world_id != _client_requested_world_id) {
    _close_client_connection("Authoritative world id mismatch");
    return;
  }

  if (_client_expected_seed > 0 && authoritative_seed != _client_expected_seed) {
    _close_client_connection("Authoritative seed mismatch");
    return;
  }

  _client_session_nonce = session_nonce;
  _client_handshake_stage = ClientHandshakeStage::WAIT_READY_ACK;
  _client_handshake_timeout_left = k_client_handshake_timeout_s;

  const Error err = rpc_id(1, "_rpc_client_ready", _client_session_nonce);
  if (err != OK) {
    _close_client_connection(
        DebugUtils::format_log("Failed sending client ready. status: %d", err));
  }
}

void NetworkManager::_rpc_client_ready(const String &session_nonce) {
  ERR_FAIL_COND_MSG(!NetUtils::is_server(this),
                    "NetworkManager: client handled client ready RPC");

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(this);
  ERR_FAIL_COND_MSG(mp.is_null(), "NetworkManager: MultiplayerAPI is null");

  const int sender_id = mp->get_remote_sender_id();
  auto it = _server_handshakes.find(sender_id);
  if (it == _server_handshakes.end() || !it->second.hello_received) {
    _disconnect_peer(sender_id, "Ready received before hello");
    return;
  }

  if (it->second.session_nonce != session_nonce) {
    _disconnect_peer(sender_id, "Ready session nonce mismatch");
    return;
  }

  _server_handshakes.erase(it);
  _mark_peer_ready(sender_id);

  const Error ack_err = rpc_id(sender_id, "_rpc_server_ready_ack", session_nonce);
  if (ack_err != OK) {
    _disconnect_peer(sender_id, DebugUtils::format_log(
                                    "Failed sending ready ack. status: %d",
                                    ack_err));
  }
}

void NetworkManager::_rpc_server_ready_ack(const String &session_nonce) {
  ERR_FAIL_COND_MSG(NetUtils::is_server(this),
                    "NetworkManager: server handled server ready ack RPC");

  if (_client_handshake_stage != ClientHandshakeStage::WAIT_READY_ACK) {
    return;
  }

  if (_client_session_nonce != session_nonce) {
    _close_client_connection("Ready ack session nonce mismatch");
    return;
  }

  _reset_client_handshake_state();
  emit_signal("connection_success");
}

void NetworkManager::_on_peer_connected(int p_peer_id) {
  if (NetUtils::is_server(this)) {
    LOG("NetworkManager: Wykryto gracza %d", p_peer_id);
    emit_signal("player_joined", p_peer_id);

    if (p_peer_id != 1) {
      ServerHandshakePeerState state;
      state.timeout_s = k_server_hello_timeout_s;
      _server_handshakes[p_peer_id] = state;
    }
  }

  connected_players[p_peer_id] = "Player_" + String::num(p_peer_id);
}

void NetworkManager::_on_peer_disconnected(int p_peer_id) {
  connected_players.erase(p_peer_id);
  ready_players.erase(p_peer_id);
  _server_handshakes.erase(p_peer_id);

  if (!NetUtils::is_server(this) && p_peer_id == 1) {
    _reset_client_handshake_state();
  }

  if (NetUtils::is_server(this)) {
    emit_signal("player_left", p_peer_id);
  }
}

void NetworkManager::_on_connected_to_server() {
  LOG("NetworkManager: connection transport established");

  const int my_id = NetUtils::get_mp(this)->get_unique_id();
  connected_players[my_id] = "Me";

  _start_client_handshake();
}

void NetworkManager::_on_connection_failed() {
  _reset_client_handshake_state();
  ERR_PRINT("NetworkManager: Nie udało się połączyć z serwerem.");
  emit_signal("connection_failed");
}

void NetworkManager::_on_server_disconnected() {
  LOG("NetworkManager: Rozłączono z serwerem.");
  connected_players.clear();
  ready_players.clear();
  _server_handshakes.clear();
  _reset_client_handshake_state();
  emit_signal("server_disconnected");
}

void NetworkManager::_reset_client_handshake_state() {
  _client_handshake_stage = ClientHandshakeStage::NONE;
  _client_handshake_timeout_left = 0.0f;
  _client_nonce = "";
  _client_session_nonce = "";
}

void NetworkManager::_disconnect_peer(int peer_id, const String &reason) {
  WARN_PRINT(String("NetworkManager: disconnect peer ") + String::num(peer_id) +
             String(" - ") + reason);

  _server_handshakes.erase(peer_id);
  ready_players.erase(peer_id);

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(this);
  if (mp.is_null() || !mp->has_multiplayer_peer()) {
    return;
  }

  Ref<MultiplayerPeer> peer = mp->get_multiplayer_peer();
  if (peer.is_valid()) {
    peer->disconnect_peer(peer_id, true);
  }
}

void NetworkManager::_close_client_connection(const String &reason) {
  ERR_PRINT(String("NetworkManager: ") + reason);

  _reset_client_handshake_state();

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(this);
  if (mp.is_valid() && mp->has_multiplayer_peer()) {
    Ref<MultiplayerPeer> peer = mp->get_multiplayer_peer();
    if (peer.is_valid()) {
      peer->close();
    }
  }

  emit_signal("connection_failed");
}

void NetworkManager::_mark_peer_ready(int peer_id) {
  if (ready_players.find(peer_id) != ready_players.end()) {
    return;
  }

  ready_players.insert(peer_id);
  emit_signal("player_ready_for_spawn", peer_id);
}

String NetworkManager::_make_server_session_nonce(int peer_id) {
  const uint64_t next = _server_next_nonce++;
  return String::num(peer_id) + "-" + String::num_uint64(next);
}

Array NetworkManager::get_ready_player_ids() const {
  Array ids;
  for (int peer_id : ready_players) {
    ids.push_back(peer_id);
  }
  return ids;
}

void NetworkManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_host", "p_port"),
                       &NetworkManager::start_host);
  ClassDB::bind_method(D_METHOD("start_client", "p_address", "p_port"),
                       &NetworkManager::start_client);
  ClassDB::bind_method(D_METHOD("configure_server_handshake_context",
                                "world_id", "seed"),
                       &NetworkManager::configure_server_handshake_context);
  ClassDB::bind_method(D_METHOD("configure_client_handshake_context",
                                "requested_world_id", "expected_seed"),
                       &NetworkManager::configure_client_handshake_context,
                       DEFVAL(0));
  ClassDB::bind_method(D_METHOD("get_ready_player_ids"),
                       &NetworkManager::get_ready_player_ids);

  ClassDB::bind_method(D_METHOD("_on_peer_connected", "p_peer_id"),
                       &NetworkManager::_on_peer_connected);
  ClassDB::bind_method(D_METHOD("_on_peer_disconnected", "p_peer_id"),
                       &NetworkManager::_on_peer_disconnected);
  ClassDB::bind_method(D_METHOD("_on_connected_to_server"),
                       &NetworkManager::_on_connected_to_server);
  ClassDB::bind_method(D_METHOD("_on_connection_failed"),
                       &NetworkManager::_on_connection_failed);
  ClassDB::bind_method(D_METHOD("_on_server_disconnected"),
                       &NetworkManager::_on_server_disconnected);

  ClassDB::bind_method(
      D_METHOD("_rpc_client_hello", "protocol_version", "client_build_hash",
               "requested_world_id", "client_nonce"),
      &NetworkManager::_rpc_client_hello);
  ClassDB::bind_method(
      D_METHOD("_rpc_server_hello_ack", "accepted", "reject_reason",
               "protocol_version", "authoritative_world_id",
               "authoritative_seed", "client_nonce", "session_nonce"),
      &NetworkManager::_rpc_server_hello_ack);
  ClassDB::bind_method(D_METHOD("_rpc_client_ready", "session_nonce"),
                       &NetworkManager::_rpc_client_ready);
  ClassDB::bind_method(D_METHOD("_rpc_server_ready_ack", "session_nonce"),
                       &NetworkManager::_rpc_server_ready_ack);

  ADD_SIGNAL(
      MethodInfo("player_joined", PropertyInfo(Variant::INT, "p_peer_id")));
  ADD_SIGNAL(
      MethodInfo("player_left", PropertyInfo(Variant::INT, "p_peer_id")));
  ADD_SIGNAL(MethodInfo("player_ready_for_spawn",
                        PropertyInfo(Variant::INT, "p_peer_id")));

  ADD_SIGNAL(MethodInfo("connection_success"));
  ADD_SIGNAL(MethodInfo("connection_failed"));
  ADD_SIGNAL(MethodInfo("server_disconnected"));
}

} // namespace morphic
