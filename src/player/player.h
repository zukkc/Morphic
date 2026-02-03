#pragma once
#include "player_terrain_viewer.h"
#include <godot_cpp/classes/character_body3d.hpp>
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

  int get_peer_id() const;
  void set_peer_id(int p_id);
  Node3D *get_head_node() const;
  double get_gravity() const;
  double get_jump_velocity() const;
  float get_speed() const;
  void set_speed(float p_speed);
  float get_friction() const;
  void set_friction(float p_friction);
  float get_sens() const;
  void set_sens(float p_sens);

private:
  int _peer_id = 1;

  Node3D *_head_node = nullptr;
  PlayerTerrainViewer _terrain_viewer;

  double _speed = 5.0;
  double _friction = 10.0;
  double _jump_velocity = 4.5;
  double _gravity = 9.8;
  float _sensitivity = 0.001;

  void setup_viewer();
  void ensure_local_controller();
};

} // namespace morphic
