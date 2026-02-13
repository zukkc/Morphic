#pragma once

#include <godot_cpp/classes/animation_node_state_machine_playback.hpp>
#include <godot_cpp/classes/animation_tree.hpp>
#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>
#include <godot_cpp/variant/vector2.hpp>

using namespace godot;

namespace morphic {

class PlayerAnimator : public Node {
  GDCLASS(PlayerAnimator, Node)

public:
  enum Action { JUMP };

protected:
  static void _bind_methods();

public:
  PlayerAnimator();
  ~PlayerAnimator();

  void _ready() override;

  void update_movement(const Vector3 &velocity, bool is_on_floor,
                       const Transform3D &player_transform,
                       float blend_max_speed);

  void trigger_action(Action action);

  void set_left_hand_item_state(String state_name, bool active);
  void set_right_hand_item_state(String state_name, bool active);
  void play_left_hand_action_state(String playback_name, String state_name);
  void play_right_hand_action_state(String playback_name, String state_name);
  void play_full_body_action_state(String state_name);

private:
  AnimationTree *_anim_tree = nullptr;
  Ref<AnimationNodeStateMachinePlayback> _main_playback;
  Ref<AnimationNodeStateMachinePlayback> _right_hand_playback;
  Ref<AnimationNodeStateMachinePlayback> _left_hand_playback;

  NodePath _anim_tree_path = NodePath("../Visuals/model/AnimationTree");

  ////////////////////////////////////////

  StringName PARAM_PLAYBACK = "parameters/playback";

  StringName STATE_LOCOMOTION = "locomotion";
  StringName PARAM_BLEND_POS = "parameters/locomotion/movement/blend_position";
  StringName PARAM_TIME_SCALE =
      "parameters/locomotion/movement_scale_time/scale";
  StringName COND_ON_FLOOR = "parameters/conditions/is_on_floor";
  StringName COND_IN_AIR = "parameters/conditions/is_in_air";
  StringName STATE_JUMP = "jump";

  StringName PARAM_LEFT_HAND = "parameters/left_hand/";
  StringName PARAM_RIGHT_HAND = "parameters/right_hand/";
  StringName PARAM_LEFT_HAND_PLAYBACK = "parameters/left_hand/playback";
  StringName PARAM_RIGHT_HAND_PLAYBACK = "parameters/right_hand/playback";

  StringName PARAM_FULL_BODY_PLAYBACK = "parameters/full_body/playback";
  StringName PARAM_FULL_BODY_ONESHOT_REQUEST =
      "parameters/full_body_one_shot/request";

  StringName PARAM_LEFT_HAND_BLEND = "parameters/left_hand_blend/blend_amount";
  StringName PARAM_RIGHT_HAND_BLEND =
      "parameters/right_hand_blend/blend_amount";

  StringName STATE_LEFT_HAND_EMPTY = "empty";
  StringName STATE_RIGHT_HAND_EMPTY = "empty";

  ///////////////////////////////////

  float _blend_smoothing = 10.0f;
  float _walk_anim_speed_ref = 2.0f;
  float _run_anim_speed_ref = 4.0f;
  float _time_scale_min = 0.5f;
  float _time_scale_max = 2.5f;
  Vector2 _current_blend = Vector2();

  Vector3 calculate_local_velocity(Vector3 global_vel,
                                   Transform3D global_transform);
  StringName get_state_name_from_action(Action action);

  void set_left_hand_item_active(bool active);
  void set_right_hand_item_active(bool active);

  // Gettery/Settery dla inspektora
  void set_anim_tree_path(NodePath p_path);
  NodePath get_anim_tree_path() const;

  void set_blend_smoothing(float p_val);
  float get_blend_smoothing() const;

  void set_walk_anim_speed_ref(float p_val);
  float get_walk_anim_speed_ref() const;

  void set_run_anim_speed_ref(float p_val);
  float get_run_anim_speed_ref() const;

  void set_time_scale_min(float p_val);
  float get_time_scale_min() const;

  void set_time_scale_max(float p_val);
  float get_time_scale_max() const;

  void set_param_blend_pos(String p_path);
  String get_param_blend_pos() const;

  void set_param_time_scale(String p_path);
  String get_param_time_scale() const;

  void set_param_playback(String p_path);
  String get_param_playback() const;

  void set_cond_in_air(String p_path);
  String get_cond_in_air() const;

  void set_cond_on_floor(String p_path);
  String get_cond_on_floor() const;

  void set_state_jump(String p_name);
  String get_state_jump() const;

  void set_state_locomotion(String p_name);
  String get_state_locomotion() const;

  void set_param_left_hand_blend(String p_name);
  String get_param_left_hand_blend() const;

  void set_param_right_hand_blend(String p_name);
  String get_param_right_hand_blend() const;

  void set_state_left_hand_empty(String p_name);
  String get_state_left_hand_empty() const;

  void set_state_right_hand_empty(String p_name);
  String get_state_right_hand_empty() const;

  void set_param_full_body_playback(String p_name);
  String get_param_full_body_playback() const;

  void set_param_full_body_oneshot_request(String p_name);
  String get_param_full_body_oneshot_request() const;
};

} // namespace morphic

VARIANT_ENUM_CAST(morphic::PlayerAnimator::Action);
