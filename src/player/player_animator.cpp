#include "player_animator.h"
#include "utils/bind_methods.h"
#include "utils/debug_utils.h"
#include <godot_cpp/classes/engine.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;

namespace morphic {

PlayerAnimator::PlayerAnimator() {}
PlayerAnimator::~PlayerAnimator() {}

void PlayerAnimator::_ready() {
  if (Engine::get_singleton()->is_editor_hint())
    return;

  Node *node = get_node_or_null(_anim_tree_path);
  if (!node) {
    ERR_PRINT(
        "PlayerAnimator: Nie znaleziono AnimationTree pod podaną ścieżką!");
    return;
  }

  _anim_tree = cast_to<AnimationTree>(node);
  if (_anim_tree) {
    _anim_tree->set_active(true);
    // Pobieramy główny playback do sterowania maszyną stanów
    _main_playback = _anim_tree->get(PARAM_PLAYBACK);
    _right_hand_playback = _anim_tree->get(PARAM_RIGHT_HAND_PLAYBACK);
    _left_hand_playback = _anim_tree->get(PARAM_LEFT_HAND_PLAYBACK);
  }
}

void PlayerAnimator::update_movement(const Vector3 &velocity, bool is_on_floor,
                                     const Transform3D &player_transform,
                                     float blend_max_speed) {
  if (!_anim_tree)
    return;

  _anim_tree->set(COND_ON_FLOOR, is_on_floor);
  _anim_tree->set(COND_IN_AIR, !is_on_floor);

  Vector3 local_vel = calculate_local_velocity(velocity, player_transform);

  Vector2 target_blend(-local_vel.z, -local_vel.x);

  if (blend_max_speed > 0.001f) {
    target_blend /= blend_max_speed;
  }

  double dt = get_physics_process_delta_time();
  _current_blend = _current_blend.lerp(
      target_blend, 1.0f - Math::exp(-_blend_smoothing * dt));

  if (_current_blend.length_squared() < 0.001f) {
    _current_blend = Vector2();
  }

  _anim_tree->set(PARAM_BLEND_POS, _current_blend);

  const float speed = Vector2(velocity.x, velocity.z).length();
  float time_scale = 1.0f;
  if (speed > 0.1f) {
    float t = 0.0f;
    if (blend_max_speed > 0.001f) {
      t = Math::clamp(speed / blend_max_speed, 0.0f, 1.0f);
    }
    const float ref = Math::lerp(_walk_anim_speed_ref, _run_anim_speed_ref, t);
    if (ref > 0.001f) {
      time_scale = speed / ref;
    }
    time_scale = Math::clamp(time_scale, _time_scale_min, _time_scale_max);
  }

  _anim_tree->set(PARAM_TIME_SCALE, time_scale);
}

void PlayerAnimator::set_left_hand_item_state(String state_name, bool active) {
  if (!active) {
    _left_hand_playback->travel(STATE_LEFT_HAND_EMPTY);
    set_left_hand_item_active(false);
    return;
  }
  if (_left_hand_playback.is_valid() && !state_name.is_empty()) {
    _left_hand_playback->travel(StringName(state_name));
  }
  set_left_hand_item_active(true);
}

void PlayerAnimator::set_right_hand_item_state(String state_name, bool active) {
  if (!active) {
    _right_hand_playback->travel(STATE_RIGHT_HAND_EMPTY);
    set_right_hand_item_active(false);
    return;
  }
  if (_right_hand_playback.is_valid() && !state_name.is_empty()) {
    _right_hand_playback->travel(StringName(state_name));
  }
  set_right_hand_item_active(true);
}

void PlayerAnimator::play_left_hand_action_state(String playback_name,
                                                 String state_name) {
  if (!_anim_tree || !_left_hand_playback.is_valid() || state_name.is_empty()) {
    return;
  }
  String playback = playback_name;
  if (playback.is_empty()) {
    playback = _left_hand_playback->get_current_node();
  }
  if (playback.is_empty()) {
    return;
  }
  const String path = String(PARAM_LEFT_HAND) + playback + "/playback";
  Variant v = _anim_tree->get(path);
  Ref<AnimationNodeStateMachinePlayback> item_playback = v;
  if (!item_playback.is_valid()) {
    return;
  }
  item_playback->travel(StringName(state_name));
}

void PlayerAnimator::play_right_hand_action_state(String playback_name,
                                                  String state_name) {
  if (!_anim_tree || !_right_hand_playback.is_valid() || state_name.is_empty()) {
    return;
  }
  String playback = playback_name;
  if (playback.is_empty()) {
    playback = _right_hand_playback->get_current_node();
  }
  if (playback.is_empty()) {
    return;
  }
  const String path = String(PARAM_RIGHT_HAND) + playback + "/playback";
  Variant v = _anim_tree->get(path);
  Ref<AnimationNodeStateMachinePlayback> item_playback = v;
  if (!item_playback.is_valid()) {
    return;
  }
  item_playback->travel(StringName(state_name));
}

void PlayerAnimator::play_full_body_action_state(String state_name) {
  if (!_anim_tree || state_name.is_empty()) {
    return;
  }
  Variant v = _anim_tree->get(PARAM_FULL_BODY_PLAYBACK);
  Ref<AnimationNodeStateMachinePlayback> playback = v;
  if (!playback.is_valid()) {
    return;
  }
  playback->travel(StringName(state_name));
  if (!String(PARAM_FULL_BODY_ONESHOT_REQUEST).is_empty()) {
    _anim_tree->set(PARAM_FULL_BODY_ONESHOT_REQUEST, 1);
  }
}

void PlayerAnimator::trigger_action(Action action) {
  if (!_main_playback.is_valid())
    return;

  StringName state_name = get_state_name_from_action(action);

  if (state_name != StringName()) {
    _main_playback->travel(state_name);
  }
}

Vector3 PlayerAnimator::calculate_local_velocity(Vector3 global_vel,
                                                 Transform3D global_transform) {
  return global_transform.get_basis().xform_inv(global_vel);
}

StringName PlayerAnimator::get_state_name_from_action(Action action) {
  switch (action) {
  case JUMP:
    return STATE_JUMP;
  default:
    return StringName();
  }
}

void PlayerAnimator::set_left_hand_item_active(bool active) {
  // Płynne przejście (możesz tu dodać Tween, albo lerp w _process)
  _anim_tree->set(PARAM_LEFT_HAND_BLEND, active ? 1.0f : 0.0f);
}

void PlayerAnimator::set_right_hand_item_active(bool active) {
  _anim_tree->set(PARAM_RIGHT_HAND_BLEND, active ? 1.0f : 0.0f);
}

// --- GETTERY / SETTERY / BINDING ---

void PlayerAnimator::set_anim_tree_path(NodePath p_path) {
  _anim_tree_path = p_path;
}
NodePath PlayerAnimator::get_anim_tree_path() const { return _anim_tree_path; }

void PlayerAnimator::set_blend_smoothing(float p_val) {
  _blend_smoothing = p_val;
}
float PlayerAnimator::get_blend_smoothing() const { return _blend_smoothing; }

void PlayerAnimator::set_walk_anim_speed_ref(float p_val) {
  _walk_anim_speed_ref = p_val;
}
float PlayerAnimator::get_walk_anim_speed_ref() const {
  return _walk_anim_speed_ref;
}

void PlayerAnimator::set_run_anim_speed_ref(float p_val) {
  _run_anim_speed_ref = p_val;
}
float PlayerAnimator::get_run_anim_speed_ref() const {
  return _run_anim_speed_ref;
}

void PlayerAnimator::set_time_scale_min(float p_val) {
  _time_scale_min = p_val;
}
float PlayerAnimator::get_time_scale_min() const { return _time_scale_min; }

void PlayerAnimator::set_time_scale_max(float p_val) {
  _time_scale_max = p_val;
}
float PlayerAnimator::get_time_scale_max() const { return _time_scale_max; }

void PlayerAnimator::set_param_blend_pos(String p_path) {
  PARAM_BLEND_POS = StringName(p_path);
}
String PlayerAnimator::get_param_blend_pos() const {
  return String(PARAM_BLEND_POS);
}

void PlayerAnimator::set_param_time_scale(String p_path) {
  PARAM_TIME_SCALE = StringName(p_path);
}
String PlayerAnimator::get_param_time_scale() const {
  return String(PARAM_TIME_SCALE);
}

void PlayerAnimator::set_param_playback(String p_path) {
  PARAM_PLAYBACK = StringName(p_path);
  if (_anim_tree) {
    _main_playback = _anim_tree->get(PARAM_PLAYBACK);
  }
}
String PlayerAnimator::get_param_playback() const {
  return String(PARAM_PLAYBACK);
}

void PlayerAnimator::set_cond_in_air(String p_path) {
  COND_IN_AIR = StringName(p_path);
}
String PlayerAnimator::get_cond_in_air() const { return String(COND_IN_AIR); }

void PlayerAnimator::set_cond_on_floor(String p_path) {
  COND_ON_FLOOR = StringName(p_path);
}
String PlayerAnimator::get_cond_on_floor() const {
  return String(COND_ON_FLOOR);
}

void PlayerAnimator::set_state_jump(String p_name) {
  STATE_JUMP = StringName(p_name);
}
String PlayerAnimator::get_state_jump() const { return String(STATE_JUMP); }

void PlayerAnimator::set_state_locomotion(String p_name) {
  STATE_LOCOMOTION = StringName(p_name);
}
String PlayerAnimator::get_state_locomotion() const {
  return String(STATE_LOCOMOTION);
}

void PlayerAnimator::set_param_left_hand_blend(String p_name) {
  PARAM_LEFT_HAND_BLEND = p_name;
}
String PlayerAnimator::get_param_left_hand_blend() const {
  return PARAM_LEFT_HAND_BLEND;
}

void PlayerAnimator::set_param_right_hand_blend(String p_name) {
  PARAM_RIGHT_HAND_BLEND = p_name;
}
String PlayerAnimator::get_param_right_hand_blend() const {
  return PARAM_RIGHT_HAND_BLEND;
}

void PlayerAnimator::set_state_left_hand_empty(String p_name) {
  STATE_LEFT_HAND_EMPTY = StringName(p_name);
}
String PlayerAnimator::get_state_left_hand_empty() const {
  return String(STATE_LEFT_HAND_EMPTY);
}

void PlayerAnimator::set_state_right_hand_empty(String p_name) {
  STATE_RIGHT_HAND_EMPTY = StringName(p_name);
}
String PlayerAnimator::get_state_right_hand_empty() const {
  return String(STATE_RIGHT_HAND_EMPTY);
}

void PlayerAnimator::set_param_full_body_playback(String p_name) {
  PARAM_FULL_BODY_PLAYBACK = StringName(p_name);
}
String PlayerAnimator::get_param_full_body_playback() const {
  return String(PARAM_FULL_BODY_PLAYBACK);
}

void PlayerAnimator::set_param_full_body_oneshot_request(String p_name) {
  PARAM_FULL_BODY_ONESHOT_REQUEST = StringName(p_name);
}
String PlayerAnimator::get_param_full_body_oneshot_request() const {
  return String(PARAM_FULL_BODY_ONESHOT_REQUEST);
}

void PlayerAnimator::_bind_methods() {
  // Metody
  ClassDB::bind_method(D_METHOD("update_movement", "velocity", "is_on_floor",
                                "player_transform", "max_speed"),
                       &PlayerAnimator::update_movement);
  ClassDB::bind_method(D_METHOD("trigger_action", "action"),
                       &PlayerAnimator::trigger_action);

  // Właściwości
  BIND_PROPERTY(PlayerAnimator, Variant::NODE_PATH, "anim_tree_path",
                anim_tree_path);
  BIND_PROPERTY(PlayerAnimator, Variant::FLOAT, "blend_smoothing",
                blend_smoothing);
  BIND_PROPERTY(PlayerAnimator, Variant::FLOAT, "walk_anim_speed_ref",
                walk_anim_speed_ref);
  BIND_PROPERTY(PlayerAnimator, Variant::FLOAT, "run_anim_speed_ref",
                run_anim_speed_ref);
  BIND_PROPERTY(PlayerAnimator, Variant::FLOAT, "time_scale_min",
                time_scale_min);
  BIND_PROPERTY(PlayerAnimator, Variant::FLOAT, "time_scale_max",
                time_scale_max);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_left_hand_blend",
                param_left_hand_blend);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_right_hand_blend",
                param_right_hand_blend);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_blend_pos",
                param_blend_pos);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_time_scale",
                param_time_scale);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_playback",
                param_playback);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "cond_on_floor",
                cond_on_floor);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "cond_in_air", cond_in_air);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "state_jump", state_jump);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "state_locomotion",
                state_locomotion);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "state_left_hand_empty",
                state_left_hand_empty);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "state_right_hand_empty",
                state_right_hand_empty);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING, "param_full_body_playback",
                param_full_body_playback);
  BIND_PROPERTY(PlayerAnimator, Variant::STRING,
                "param_full_body_oneshot_request",
                param_full_body_oneshot_request);

  // Binding Enuma dla Inspektora i Skryptów
  BIND_ENUM_CONSTANT(JUMP);
}

} // namespace morphic
