#pragma once
#include <godot_cpp/classes/character_body3d.hpp>
#include <godot_cpp/classes/input.hpp>
#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node3d.hpp>

namespace morphic {

class Player : public godot::CharacterBody3D {
  GDCLASS(Player, godot::CharacterBody3D)

protected:
  static void _bind_methods();

public:
  Player();

  void _enter_tree() override;
  void _ready() override;
  void _physics_process(double delta) override;
  void _input(const godot::Ref<godot::InputEvent> &event) override;

  int get_peer_id() const;
  void set_peer_id(int p_id);

private:
  int _peer_id = 1;

  godot::Node3D *_head_node;

  double _speed = 5.0;
  double _friction = 10.0;
  double _jump_velocity = 4.5;
  double _gravity = 9.8;
  float _sensitivity = 0.001;

  void setup_viewer();
  void setup_player_camera();

  float get_speed() const;
  void set_speed(float p_speed);
  float get_friction() const;
  void set_friction(float p_friction);
  float get_sens() const;
  void set_sens(float p_friction);
};

} // namespace morphic
