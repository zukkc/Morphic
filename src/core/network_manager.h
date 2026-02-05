#pragma once
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/dictionary.hpp>
#include <unordered_map>
#include <unordered_set>

using namespace godot;

namespace morphic {

class NetworkManager : public Node {
  GDCLASS(NetworkManager, Node)

private:
  enum class ClientHandshakeStage {
    NONE = 0,
    WAIT_HELLO_ACK = 1,
    WAIT_READY_ACK = 2
  };

  struct ServerHandshakePeerState {
    float timeout_s = 0.0f;
    bool hello_received = false;
    bool ready_received = false;
    int protocol_version = 0;
    String client_build_hash;
    String requested_world_id;
    String client_nonce;
    String session_nonce;
  };

  static constexpr int k_protocol_version = 1;
  static constexpr float k_server_hello_timeout_s = 5.0f;
  static constexpr float k_server_ready_timeout_s = 10.0f;
  static constexpr float k_client_handshake_timeout_s = 10.0f;

  Dictionary connected_players;
  std::unordered_set<int> ready_players;
  std::unordered_map<int, ServerHandshakePeerState> _server_handshakes;

  String _server_world_id;
  int _server_seed = 0;

  String _client_requested_world_id;
  int _client_expected_seed = 0;
  String _client_build_hash = "dev";
  String _client_nonce;
  String _client_session_nonce;
  float _client_handshake_timeout_left = 0.0f;
  ClientHandshakeStage _client_handshake_stage = ClientHandshakeStage::NONE;

  uint64_t _server_next_nonce = 1;

  void _reset_client_handshake_state();
  void _start_client_handshake();
  void _disconnect_peer(int peer_id, const String &reason);
  void _close_client_connection(const String &reason);
  void _mark_peer_ready(int peer_id);
  String _make_server_session_nonce(int peer_id);

  void _rpc_client_hello(int protocol_version, const String &client_build_hash,
                         const String &requested_world_id,
                         const String &client_nonce);
  void _rpc_server_hello_ack(bool accepted, const String &reject_reason,
                             int protocol_version,
                             const String &authoritative_world_id,
                             int authoritative_seed,
                             const String &client_nonce,
                             const String &session_nonce);
  void _rpc_client_ready(const String &session_nonce);
  void _rpc_server_ready_ack(const String &session_nonce);

protected:
  static void _bind_methods();

public:
  void _ready() override;
  void _process(double delta) override;

  bool start_host(int port);
  bool start_client(const String &address, int port);
  void configure_server_handshake_context(const String &world_id, int seed);
  void configure_client_handshake_context(const String &requested_world_id,
                                          int expected_seed = 0);

  void _on_peer_connected(int peer_id);
  void _on_peer_disconnected(int peer_id);
  void _on_connected_to_server();
  void _on_connection_failed();
  void _on_server_disconnected();

  inline Dictionary get_player_list() const { return connected_players; }
  Array get_ready_player_ids() const;
  int get_protocol_version() const { return k_protocol_version; }
};

} // namespace morphic
