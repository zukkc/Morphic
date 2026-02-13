#include "player_input.h"

#include "godot_cpp/classes/input.hpp"
#include "godot_cpp/classes/input_event_mouse_button.hpp"
#include "godot_cpp/classes/input_event_mouse_motion.hpp"

namespace morphic {
PlayerInputState PlayerInput::consume() {
  auto state = _state;
  _state = PlayerInputState();
  return state;
}

void PlayerInput::poll_actions() {
  Input *input = Input::get_singleton();
  _state.move = input->get_vector("move_left", "move_right", "move_forward",
                                  "move_backward");
  _state.jump = input->is_action_just_pressed("jump");
  _state.is_sprinting = input->is_action_pressed("sprint");
  _state.primary_action = input->is_action_pressed("primary_action");
  _state.secondary_action = input->is_action_pressed("secondary_action");
  _state.toggle_torch = input->is_action_just_pressed("toggle_torch");
  _state.toggle_picaxe = input->is_action_just_pressed("toggle_picaxe");
}

void PlayerInput::handle_input(const Ref<InputEvent> &event) {
  Input *input = Input::get_singleton();

  Ref<InputEventMouseButton> mb = event;
  if (mb.is_valid() && mb->is_pressed() &&
      mb->get_button_index() == MouseButton::MOUSE_BUTTON_LEFT) {
    if (input->get_mouse_mode() != Input::MOUSE_MODE_CAPTURED) {
      input->set_mouse_mode(Input::MOUSE_MODE_CAPTURED);
      return;
    }
  }

  if (event->is_action_pressed("ui_cancel")) {
    input->set_mouse_mode(Input::MOUSE_MODE_VISIBLE);
  }

  Ref<InputEventMouseMotion> mouse_motion = event;
  if (mouse_motion.is_valid() &&
      input->get_mouse_mode() == Input::MOUSE_MODE_CAPTURED) {
    _state.look += mouse_motion->get_relative();
  }
}
} // namespace morphic
