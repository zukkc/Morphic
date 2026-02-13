#include "player.h"
#include "local_player_controller.h"
#include "player_equipment.h"
#include "utils/bind_methods.h"
#include "utils/debug_utils.h"

#include <godot_cpp/classes/camera3d.hpp>
#include <godot_cpp/classes/engine.hpp>

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

void Player::notify_jump() {
  if (!_player_animator) {
    return;
  }
  _player_animator->trigger_action(PlayerAnimator::JUMP);
}

void Player::toggle_torch() {
  if (!_equipment) {
    return;
  }
  int torch_id = _equipment->get_item_id_by_name("torch");
  if (torch_id <= 0) {
    torch_id = 1;
  }
  const int current_left = _equipment->get_left_item();
  if (current_left == torch_id) {
    _equipment->clear_left();
  } else {
    _equipment->equip_left(torch_id);
  }
}

void Player::toggle_picaxe() {
  if (!_equipment) {
    return;
  }
  int picaxe_id = _equipment->get_item_id_by_name("picaxe");
  if (picaxe_id <= 0) {
    picaxe_id = 2;
  }
  const int current_right = _equipment->get_right_item();
  if (current_right == picaxe_id) {
    _equipment->clear_right();
  } else {
    _equipment->equip_right(picaxe_id);
  }
}

void Player::trigger_left_item_action(String action) {
  if (!_equipment || !_player_animator) {
    return;
  }
  Ref<ItemDefinition> item = _equipment->get_left_item_def();
  if (item.is_null()) {
    return;
  }
  const String full_body = item->get_full_body_action(action);
  if (!full_body.is_empty()) {
    _player_animator->play_full_body_action_state(full_body);
    return;
  }
  const String hand_state = item->get_hand_action(action);
  if (hand_state.is_empty()) {
    return;
  }
  _player_animator->play_left_hand_action_state(item->get_hand_playback(),
                                                hand_state);
}

void Player::trigger_right_item_action(String action) {
  if (!_equipment || !_player_animator) {
    return;
  }
  Ref<ItemDefinition> item = _equipment->get_right_item_def();
  if (item.is_null()) {
    return;
  }
  const String full_body = item->get_full_body_action(action);
  if (!full_body.is_empty()) {
    _player_animator->play_full_body_action_state(full_body);
    return;
  }
  const String hand_state = item->get_hand_action(action);
  if (hand_state.is_empty()) {
    return;
  }
  _player_animator->play_right_hand_action_state(item->get_hand_playback(),
                                                 hand_state);
}

void Player::_enter_tree() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }
  _terrain_viewer.setup_viewer(this);
}

void Player::_ready() {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

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
  } else {
    ensure_local_controller();
  }

  Node *player_animator_node = get_node_or_null(_player_animator_path);
  ERR_FAIL_COND_MSG(!player_animator_node,
                    "Cant find PlayerAnimator node. Check if path is correct");
  _player_animator = cast_to<PlayerAnimator>(player_animator_node);
  ERR_FAIL_COND_MSG(!_player_animator, "Cast to PlayerAnimator failed.");

  Node *equipment_node = get_node_or_null(_equipment_path);
  if (!equipment_node) {
    ERR_PRINT("Cant find PlayerEquipment node. Check if path is correct");
  } else {
    _equipment = cast_to<PlayerEquipment>(equipment_node);
    ERR_FAIL_COND_MSG(!_equipment, "Cast to PlayerEquipment failed.");

    Error err = _equipment->connect("right_item_equipped",
                                    Callable(this, "on_right_hand_equipped"));
    if (err != OK) {
      ERR_PRINT("Player: connect right_item_equipped failed.");
    }
    err = _equipment->connect("left_item_equipped",
                              Callable(this, "on_left_hand_equipped"));
    if (err != OK) {
      ERR_PRINT("Player: connect left_item_equipped failed.");
    }
  }

  Node *left_hand_socket_node = get_node_or_null(_left_hand_socket_path);
  ERR_FAIL_COND_MSG(
      !left_hand_socket_node,
      "Cant find left hand socket node. Check if path is correct");
  _left_hand_socket = cast_to<Marker3D>(left_hand_socket_node);
  ERR_FAIL_COND_MSG(!_left_hand_socket, "Failed casting to Left Hand Socked");

  Node *right_hand_socket_node = get_node_or_null(_right_hand_socket_path);
  ERR_FAIL_COND_MSG(
      !right_hand_socket_node,
      "Cant find right hand socket node. Check if path is correct");
  _right_hand_socket = cast_to<Marker3D>(right_hand_socket_node);
  ERR_FAIL_COND_MSG(!_right_hand_socket, "Failed casting to Right Hand Socked");

  // for late join player to sync items in hands
  apply_current_equipment();
}

void Player::_physics_process(double) {
  if (Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (!_player_animator)
    return;

  const bool anim_on_floor = is_multiplayer_authority() ? is_on_floor() : true;
  const float blend_max_speed = _sprint_speed;
  const float time_scale_ref_speed =
      (_movement_ref_speed > 0.001f) ? _movement_ref_speed : blend_max_speed;
  _player_animator->update_movement(get_velocity(), anim_on_floor,
                                    get_transform(), blend_max_speed);
}

// private

void Player::on_right_hand_equipped(Ref<ItemDefinition> item) {
  if (_right_hand_item) {
    _right_hand_item->queue_free();
    _right_hand_item = nullptr;
  }
  if (!item.is_valid()) {
    _player_animator->set_right_hand_item_state("", false);
    return;
  }

  Ref<PackedScene> scene = item->get_model_scene();
  if (scene.is_valid() && _right_hand_socket) {
    Node *inst = scene->instantiate();
    _right_hand_item = Object::cast_to<Node3D>(inst);
    if (_right_hand_item) {
      _right_hand_socket->add_child(_right_hand_item);
    } else {
      inst->queue_free();
    }
  }
  _player_animator->set_right_hand_item_state(item->get_equip_state(), true);
}

void Player::on_left_hand_equipped(Ref<ItemDefinition> item) {
  if (_left_hand_item) {
    _left_hand_item->queue_free();
    _left_hand_item = nullptr;
  }

  if (!item.is_valid()) {
    _player_animator->set_left_hand_item_state("", false);
    return;
  }

  Ref<PackedScene> scene = item->get_model_scene();
  if (scene.is_valid() && _left_hand_socket) {
    Node *inst = scene->instantiate();
    _left_hand_item = Object::cast_to<Node3D>(inst);
    if (_left_hand_item) {
      _left_hand_socket->add_child(_left_hand_item);
    } else {
      inst->queue_free();
    }
  }

  _player_animator->set_left_hand_item_state(item->get_equip_state(), true);
}

void Player::apply_current_equipment() {
  Ref<ItemDatabase> db = _equipment->get_item_database();

  Ref<ItemDefinition> left;
  Ref<ItemDefinition> right;

  if (db.is_valid()) {
    left = db->get_item_by_id(_equipment->get_left_item());
    right = db->get_item_by_id(_equipment->get_right_item());
  }

  on_left_hand_equipped(left);
  on_right_hand_equipped(right);
}

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

NodePath Player::get_player_animator_path() const {
  return _player_animator_path;
}
void Player::set_player_animator_path(NodePath p_path) {
  _player_animator_path = p_path;
}

NodePath Player::get_equipment_path() const { return _equipment_path; }
void Player::set_equipment_path(NodePath p_path) { _equipment_path = p_path; }

NodePath Player::get_left_hand_socket_path() const {
  return _left_hand_socket_path;
}
void Player::set_left_hand_socket_path(NodePath p_path) {
  _left_hand_socket_path = p_path;
}
NodePath Player::get_right_hand_socket_path() const {
  return _right_hand_socket_path;
}
void Player::set_right_hand_socket_path(NodePath p_path) {
  _right_hand_socket_path = p_path;
}

float Player::get_sprint_speed() const { return _sprint_speed; }
void Player::set_sprint_speed(float p_speed) { _sprint_speed = p_speed; }

float Player::get_speed() const { return _speed; }
void Player::set_speed(float p_speed) { _speed = p_speed; }

float Player::get_friction() const { return _friction; }
void Player::set_friction(float p_friction) { _friction = p_friction; }

float Player::get_sensitivity() const { return _sensitivity; }
void Player::set_sensitivity(float p_sens) { _sensitivity = p_sens; }

float Player::get_movement_ref_speed() const { return _movement_ref_speed; }
void Player::set_movement_ref_speed(float p_speed) {
  _movement_ref_speed = p_speed;
}

void Player::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_right_hand_equipped", "item"),
                       &Player::on_right_hand_equipped);
  ClassDB::bind_method(D_METHOD("on_left_hand_equipped", "item"),
                       &Player::on_left_hand_equipped);
  ClassDB::bind_method(D_METHOD("trigger_left_item_action", "action"),
                       &Player::trigger_left_item_action);
  ClassDB::bind_method(D_METHOD("trigger_right_item_action", "action"),
                       &Player::trigger_right_item_action);

  BIND_PROPERTY(Player, Variant::NODE_PATH, "player_animator_path",
                player_animator_path);
  BIND_PROPERTY(Player, Variant::NODE_PATH, "equipment_path", equipment_path);
  BIND_PROPERTY(Player, Variant::NODE_PATH, "left_hand_socket_path",
                left_hand_socket_path);
  BIND_PROPERTY(Player, Variant::NODE_PATH, "right_hand_socket_path",
                right_hand_socket_path);
  BIND_PROPERTY(Player, Variant::FLOAT, "sensitivity", sensitivity);
  BIND_PROPERTY(Player, Variant::FLOAT, "sprint_speed", sprint_speed);
  BIND_PROPERTY(Player, Variant::FLOAT, "speed", speed);
  BIND_PROPERTY(Player, Variant::FLOAT, "friction", friction);
}

} // namespace morphic
