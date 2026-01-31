#include "player.h"
#include "utils/network_utils.h"

#include "godot_cpp/classes/input_event_mouse_motion.hpp"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/classes/multiplayer_api.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/voxel_viewer.hpp>

using namespace godot;

namespace morphic {

Player::Player() {
  // Inicjalizacja zmiennych
}

int Player::get_peer_id() const { return _peer_id; }
void Player::set_peer_id(int p_peer_id) { _peer_id = p_peer_id; }

void Player::_enter_tree() {
  if (_peer_id == 0) {
    String name = get_name();
    if (name.is_valid_int()) {
      _peer_id = name.to_int();
    }
  }

  set_multiplayer_authority(_peer_id);
  setup_viewer();
}

void Player::setup_viewer() {
  Node *existing = get_node_or_null("VoxelViewer");
  if (existing) {
    existing->queue_free();
  }

  bool is_server_inst = NetUtils::is_server(this);
  bool needs_viewer = false;

  if (is_server_inst) {
    needs_viewer = true;
  } else if (is_multiplayer_authority()) {
    needs_viewer = true;
  }

  if (!needs_viewer)
    return;

  VoxelViewer *viewer = memnew(VoxelViewer);
  viewer->set_name("VoxelViewer");
  add_child(viewer);

  if (is_server_inst) {
    if (_peer_id != 1) {
      viewer->set_network_peer_id(_peer_id);
      viewer->set_requires_data_block_notifications(true);
      viewer->set_requires_visuals(false);
    } else {
      viewer->set_requires_visuals(true);
    }
    viewer->set_requires_collisions(true);
  } else {
    viewer->set_requires_visuals(true);
    viewer->set_requires_collisions(true);
  }
}

void Player::_ready() {
  _head_node = get_node<Node3D>("Head");

  // Złap myszkę
  Input::get_singleton()->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
}

void Player::_physics_process(double delta) {
  // Nie wykonuj ruchu w edytorze!
  if (Engine::get_singleton()->is_editor_hint())
    return;

  if (!is_multiplayer_authority())
    return;

  Vector3 velocity = get_velocity();

  // 1. Grawitacja
  if (!is_on_floor()) {
    velocity.y -= _gravity * delta;
  }

  // 2. Skakanie
  Input *input = Input::get_singleton();
  if (input->is_action_just_pressed("jump") && is_on_floor()) {
    velocity.y = _jump_velocity;
  }

  // 3. Ruch (WASD)
  Vector2 input_dir = input->get_vector("move_left", "move_right",
                                        "move_forward", "move_backward");
  Vector3 direction = get_transform()
                          .basis.xform(Vector3(input_dir.x, 0, input_dir.y))
                          .normalized();
  if (direction != Vector3(0, 0, 0)) {
    velocity.x = direction.x * _speed;
    velocity.z = direction.z * _speed;
  } else {
    // Hamowanie
    velocity.x =
        Math::move_toward(velocity.x, 0, (real_t)(_speed * _friction * delta));
    velocity.z =
        Math::move_toward(velocity.z, 0, (real_t)(_speed * _friction * delta));
  }

  set_velocity(velocity);

  // Kluczowa funkcja - wbudowana w klasę
  move_and_slide();
}

void Player::_input(const Ref<InputEvent> &event) {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  Input *input = Input::get_singleton();

  if (event->is_action_pressed("toggle_mouse")) { // np. pod TAB
    if (input->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
      // Uwolnij myszkę
      input->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
    } else {
      // Złap myszkę z powrotem
      input->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
    }
  }

  // Przykład obsługi myszki (obracanie kamerą):
  InputEventMouseMotion *mouse_motion =
      Object::cast_to<InputEventMouseMotion>(*event);
  if (mouse_motion && input->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
    // Twoja logika obracania głową
    rotate_y(-mouse_motion->get_relative().x * _sensitivity);
    _head_node->rotate_x(-mouse_motion->get_relative().y * _sensitivity);
  }
}

// private

float Player::get_speed() const { return _speed; }
void Player::set_speed(float p_speed) { _speed = p_speed; }

float Player::get_friction() const { return _friction; }
void Player::set_friction(float p_friction) { _friction = p_friction; }

float Player::get_sens() const { return _sensitivity; }
void Player::set_sens(float p_sens) { _sensitivity = p_sens; }

void Player::_bind_methods() {

  ClassDB::bind_method(D_METHOD("get_sens"), &Player::get_sens);
  ClassDB::bind_method(D_METHOD("set_sens", "p_sens"), &Player::set_sens);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "sensitivity"), "set_sens",
               "get_sens");

  ClassDB::bind_method(D_METHOD("get_speed"), &Player::get_speed);
  ClassDB::bind_method(D_METHOD("set_speed", "p_speed"), &Player::set_speed);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "speed"), "set_speed", "get_speed");

  ClassDB::bind_method(D_METHOD("get_friction"), &Player::get_friction);
  ClassDB::bind_method(D_METHOD("set_friction", "p_priction"),
                       &Player::set_friction);
  ADD_PROPERTY(PropertyInfo(Variant::FLOAT, "friction"), "set_friction",
               "get_friction");
}

} // namespace morphic
