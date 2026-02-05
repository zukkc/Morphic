#pragma once
#include "utils/debug_utils.h"
#include <core/network_manager.h>

#include "godot_cpp/classes/display_server.hpp"
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/classes/scene_tree.hpp>

using namespace godot;

namespace morphic {

namespace NetUtils {

inline Ref<MultiplayerAPI> get_mp(const Node *context) {
  if (context == nullptr) {
    ERR_PRINT("NetUtils: Context is null!");
    return Ref<MultiplayerAPI>();
  }

  SceneTree *tree = context->get_tree();
  if (tree == nullptr) {
    ERR_PRINT("NetUtils: Node is not inside SceneTree!");
    return Ref<MultiplayerAPI>();
  }

  return tree->get_multiplayer();
}

inline bool is_server(const Node *context) {
  Ref<MultiplayerAPI> mp = get_mp(context);
  if (mp.is_valid() && mp->has_multiplayer_peer()) {
    return mp->is_server();
  }
  return false;
}

inline DisplayServer *get_server() { return DisplayServer::get_singleton(); }

inline bool is_context_valid(const Node *context) {
  if (context == nullptr) {
    ERR_PRINT("NetUtils: Context is null!");
    return false;
  }
  return true;
}

inline bool is_headless() {
  godot::DisplayServer *ds = godot::DisplayServer::get_singleton();
  if (ds) {
    return ds->get_name() == "headless";
  }
  return false;
}

inline NetworkManager *get_net_manager(const Node *context) {
  if (context == nullptr) {
    ERR_PRINT("NetUtils: Context is null!");
    return nullptr;
  }

  NetworkManager *net_manager = Object::cast_to<NetworkManager>(
      context->get_node_or_null("/root/GlobalNetworkManager"));
  if (!net_manager) {
    ERR_PRINT("CRITICAL: Brak NetworkManagera w /root!");
    return nullptr;
  }

  return net_manager;
}
}; // namespace NetUtils

} // namespace morphic
