#include "player_terrain_viewer.h"
#include "godot_cpp/classes/voxel_viewer.hpp"
#include "player.h"
#include "utils/debug_utils.h"
#include "utils/network_utils.h"

using namespace godot;

namespace morphic {
void PlayerTerrainViewer::setup_viewer(Node *p_player_node) {
  if (p_player_node == nullptr) {
    ERR_PRINT("player is null");
    return;
  }

  Player *player = player->cast_to<Player>(p_player_node);

  if (player == nullptr) {
    ERR_PRINT("Casting to player failed");
    return;
  }

  Node *existing = player->get_node_or_null("TerrrainViewer");
  if (existing) {
    existing->queue_free();
  }

  Ref<MultiplayerAPI> mp = NetUtils::get_mp(player);
  const bool has_mp = mp.is_valid() && mp->has_multiplayer_peer();
  const bool is_server_inst = has_mp && mp->is_server();
  const int local_id = has_mp ? mp->get_unique_id() : 0;
  const bool is_local_player = (player->get_peer_id() == local_id);

  const bool needs_viewer =
      is_server_inst || player->is_multiplayer_authority();
  if (!needs_viewer) {
    return;
  }

  VoxelViewer *viewer = memnew(VoxelViewer);
  viewer->set_name("TerrainViewer");
  player->add_child(viewer);

  if (is_server_inst) {
    viewer->set_network_peer_id(player->get_peer_id());
    viewer->set_requires_data_block_notifications(true);
    viewer->set_requires_visuals(is_local_player); // tylko lokalny gracz
    viewer->set_requires_collisions(true);
  } else {
    const bool local = player->is_multiplayer_authority();
    viewer->set_requires_visuals(local);
    viewer->set_requires_collisions(local);
  }
}
} // namespace morphic
