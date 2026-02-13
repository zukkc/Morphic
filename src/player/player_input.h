#pragma once

#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/variant/vector2.hpp"

using namespace godot;

namespace morphic {

struct PlayerInputState {
  Vector2 move;
  Vector2 look;
  bool primary_action = false;
  bool secondary_action = false;
  bool jump = false;
  bool is_sprinting = false;
  bool toggle_torch = false;
  bool toggle_picaxe = false;
};

class PlayerInput {
public:
  void handle_input(const Ref<InputEvent> &event);
  void poll_actions();
  PlayerInputState consume();

private:
  PlayerInputState _state;
};

} // namespace morphic
