#include "player.h"
#include "local_player_controller.h"
#include "utils/bind_methods.h"

#include <godot_cpp/classes/camera3d.hpp>

using namespace godot;

namespace morphic {

Player::Player() {
  // Inicjalizacja zmiennych
}

int Player::get_peer_id() const { return _peer_id; }
void Player::set_peer_id(int p_peer_id) { _peer_id = p_peer_id; }
Node3D *Player::get_head_node() const { return _head_node; }
double Player::get_gravity() const { return _gravity; }
double Player::get_jump_velocity() const { return _jump_velocity; }

void Player::_enter_tree() { _terrain_viewer.setup_viewer(this); }

void Player::_ready() {
  Node *head_node = get_node_or_null("Head");
  _head_node = Object::cast_to<Node3D>(head_node);

  if (!is_multiplayer_authority()) {
    if (_head_node) {
      Node *camera_node = _head_node->find_child("Camera3D");
      Camera3D *camera = Object::cast_to<Camera3D>(camera_node);
      if (camera) {
        camera->set_current(false);
      }
    }
    return;
  }

  ensure_local_controller();
}

// private

void Player::ensure_local_controller() {
  if (!is_multiplayer_authority()) {
    return;
  }

  Node *existing = get_node_or_null("LocalPlayerController");
  if (existing) {
    return;
  }

  LocalPlayerController *controller = memnew(LocalPlayerController);
  controller->set_name("LocalPlayerController");
  add_child(controller);
}

float Player::get_speed() const { return _speed; }
void Player::set_speed(float p_speed) { _speed = p_speed; }

float Player::get_friction() const { return _friction; }
void Player::set_friction(float p_friction) { _friction = p_friction; }

float Player::get_sens() const { return _sensitivity; }
void Player::set_sens(float p_sens) { _sensitivity = p_sens; }

void Player::_bind_methods() {

  BIND_PROPERTY(Player, Variant::FLOAT, "sensitivity", sens);
  BIND_PROPERTY(Player, Variant::FLOAT, "speed", speed);
  BIND_PROPERTY(Player, Variant::FLOAT, "friction", friction);
}

} // namespace morphic
