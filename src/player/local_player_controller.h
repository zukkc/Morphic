#pragma once

#include "player_camera.h"
#include "player_input.h"
#include "player_movement.h"

#include <godot_cpp/classes/input_event.hpp>
#include <godot_cpp/classes/node.hpp>

namespace morphic {

class Player;

class LocalPlayerController : public godot::Node {
  GDCLASS(LocalPlayerController, godot::Node)

protected:
  static void _bind_methods();

public:
  void _ready() override;
  void _input(const godot::Ref<godot::InputEvent> &event) override;
  void _physics_process(double delta) override;

private:
  Player *_player = nullptr;
  bool _camera_ready = false;

  PlayerInput _input_state;
  PlayerMovement _movement;
  PlayerCamera _camera;
};

} // namespace morphic
