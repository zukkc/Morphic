#include "utils/debug_utils.h"
#include "utils/network_utils.h"

#include <godot_cpp/classes/e_net_multiplayer_peer.hpp>

using namespace godot;

namespace morphic {

void NetworkManager::_ready() {
  Ref<MultiplayerAPI> mp = get_tree()->get_multiplayer();
  mp->connect("peer_connected", Callable(this, "_on_peer_connected"));
  mp->connect("peer_disconnected", Callable(this, "_on_peer_disconnected"));
  mp->connect("connected_to_server", Callable(this, "_on_connected_to_server"));
  mp->connect("connection_failed", Callable(this, "_on_connection_failed"));
  mp->connect("server_disconnected", Callable(this, "_on_server_disconnected"));
}

bool NetworkManager::start_host(int port) {
  Ref<ENetMultiplayerPeer> peer;
  peer.instantiate();
  Error err = peer->create_server(port);
  if (err != Error::OK) {
    ERR_PRINT(DebugUtils::format_log(
        "Error: failed creating the server. status: %d", err));
    return false;
  } else {
    NetUtils::get_mp(this)->set_multiplayer_peer(peer);
    connected_players[1] = "Host";
    emit_signal("player_joined", 1);
    LOG("Successfuly created the server");
    return true;
  }
}

bool NetworkManager::start_client(const String &p_address, int p_port) {
  Ref<ENetMultiplayerPeer> peer;
  peer.instantiate();
  Error err = peer->create_client(p_address, p_port);
  if (err != Error::OK) {
    ERR_PRINT(DebugUtils::format_log(
        "Error: failed joining to the server. status: %d", err));
    return false;
  } else {
    NetUtils::get_mp(this)->set_multiplayer_peer(peer);
    LOG("Successfuly joined the  server");
    return true;
  }
}

void NetworkManager::_on_peer_connected(int p_peer_id) {
  if (NetUtils::is_server(this))
    LOG("NetworkManager: Wykryto gracza %d", p_peer_id);
  connected_players[p_peer_id] = "Player_" + String::num(p_peer_id);

  emit_signal("player_joined", p_peer_id);
}

void NetworkManager::_on_peer_disconnected(int p_peer_id) {
  connected_players.erase(p_peer_id);
  emit_signal("player_left", p_peer_id);
}

void NetworkManager::_on_connected_to_server() {
  LOG("NetworkManager: Pomyślnie połączono z serwerem! (To jestem JA)");

  // Pobierz swoje ID
  int my_id = NetUtils::get_mp(this)->get_unique_id();
  connected_players[my_id] = "Me"; // Lub nazwa gracza

  emit_signal("connection_success");
}

void NetworkManager::_on_connection_failed() {
  ERR_PRINT("NetworkManager: Nie udało się połączyć z serwerem.");
  emit_signal("connection_failed");
}

void NetworkManager::_on_server_disconnected() {
  LOG("NetworkManager: Rozłączono z serwerem.");
  connected_players.clear();
  emit_signal("server_disconnected");
}

void NetworkManager::_bind_methods() {
  ClassDB::bind_method(D_METHOD("start_host", "p_port"),
                       &NetworkManager::start_host);
  ClassDB::bind_method(D_METHOD("start_client", "p_address", "p_port"),
                       &NetworkManager::start_client);
  ClassDB::bind_method(D_METHOD("_on_peer_connected", "p_peer_id"),
                       &NetworkManager::_on_peer_connected);
  ClassDB::bind_method(D_METHOD("_on_peer_disconnected", "p_peer_id"),
                       &NetworkManager::_on_peer_disconnected);

  ADD_SIGNAL(
      MethodInfo("player_joined", PropertyInfo(Variant::INT, "p_peer_id")));
  ADD_SIGNAL(
      MethodInfo("player_left", PropertyInfo(Variant::INT, "p_peer_id")));

  ClassDB::bind_method(D_METHOD("_on_connected_to_server"),
                       &NetworkManager::_on_connected_to_server);
  ClassDB::bind_method(D_METHOD("_on_connection_failed"),
                       &NetworkManager::_on_connection_failed);
  ClassDB::bind_method(D_METHOD("_on_server_disconnected"),
                       &NetworkManager::_on_server_disconnected);

  ADD_SIGNAL(MethodInfo("connection_success"));
  ADD_SIGNAL(MethodInfo("connection_failed"));
  ADD_SIGNAL(MethodInfo("server_disconnected"));
}

} // namespace morphic
