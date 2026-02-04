#pragma once
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/node.hpp>

using namespace godot;

namespace morphic {

class NetworkManager : public Node {
  GDCLASS(NetworkManager, Node)

private:
  Dictionary connected_players;

protected:
  static void _bind_methods();

public:
  void _ready() override;

  bool start_host(int port);
  bool start_client(const String &address, int port);

  void _on_peer_connected(int peer_id);
  void _on_peer_disconnected(int peer_id);
  void _on_connected_to_server();
  void _on_connection_failed();
  void _on_server_disconnected();

  inline Dictionary get_player_list() const { return connected_players; }
};

} // namespace morphic
