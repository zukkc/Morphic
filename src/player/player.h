#pragma once

#include "player_animator.h"
#include "player_equipment.h"
#include "player_terrain_viewer.h"

#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/marker3d.hpp>
#include <godot_cpp/classes/node3d.hpp>

using namespace godot;

namespace morphic {

class Player : public CharacterBody3D {
  GDCLASS(Player, CharacterBody3D)

protected:
  static void _bind_methods();

public:
  Player();

  void _enter_tree() override;
  void _ready() override;
  void _physics_process(double delta) override;

  NodePath get_player_animator_path() const;
  void set_player_animator_path(NodePath p_path);
  NodePath get_equipment_path() const;
  void set_equipment_path(NodePath p_path);
  NodePath get_left_hand_socket_path() const;
  void set_left_hand_socket_path(NodePath p_path);
  NodePath get_right_hand_socket_path() const;
  void set_right_hand_socket_path(NodePath p_path);
  int get_peer_id() const;
  void set_peer_id(int p_id);
  Node3D *get_head_node() const;
  double get_gravity() const;
  double get_jump_velocity() const;
  float get_sprint_speed() const;
  void set_sprint_speed(float p_speed);
  float get_speed() const;
  void set_speed(float p_speed);
  float get_friction() const;
  void set_friction(float p_friction);
  float get_sensitivity() const;
  void set_sensitivity(float p_sens);
  float get_movement_ref_speed() const;
  void set_movement_ref_speed(float p_speed);

  void notify_jump();
  void toggle_torch();
  void toggle_picaxe();
  void trigger_left_item_action(String action);
  void trigger_right_item_action(String action);

private:
  int _peer_id = 1;

  Node3D *_head_node = nullptr;
  PlayerTerrainViewer _terrain_viewer;
  PlayerAnimator *_player_animator = nullptr;

  PlayerEquipment *_equipment = nullptr;
  Marker3D *_left_hand_socket = nullptr;
  Marker3D *_right_hand_socket = nullptr;
  Node3D *_left_hand_item = nullptr;
  Node3D *_right_hand_item = nullptr;

  NodePath _player_animator_path;
  NodePath _equipment_path = NodePath("Equipment");
  NodePath _left_hand_socket_path;
  NodePath _right_hand_socket_path;

  float _sprint_speed = 4;
  float _speed = 2;
  float _friction = 10.0;
  float _jump_velocity = 4.5;
  float _gravity = 9.8;
  float _sensitivity = 0.001;
  float _movement_ref_speed = 0.0f;

  void setup_viewer();
  void ensure_local_controller();
  void on_left_hand_equipped(Ref<ItemDefinition> p_item);
  void on_right_hand_equipped(Ref<ItemDefinition> p_item);
  void apply_current_equipment();
};

} // namespace morphic
