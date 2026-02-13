#include "local_player_controller.h"
#include "player.h"

#include <godot_cpp/classes/engine.hpp>

namespace morphic {

void LocalPlayerController::_bind_methods() {}

void LocalPlayerController::_ready() {
  _player = godot::Object::cast_to<Player>(get_parent());
  if (!_player) {
    ERR_PRINT("LocalPlayerController must be a child of Player");
    set_process(false);
    set_physics_process(false);
    set_process_input(false);
    return;
  }

  if (!_player->is_multiplayer_authority()) {
    set_process(false);
    set_physics_process(false);
    set_process_input(false);
    return;
  }

  set_process_input(true);
  set_physics_process(true);

  _camera_ready = _camera.setup(_player, _player->get_head_node());
}

void LocalPlayerController::_input(const godot::Ref<godot::InputEvent> &event) {
  if (godot::Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (!_player || !_player->is_multiplayer_authority()) {
    return;
  }

  _input_state.handle_input(event);
}

void LocalPlayerController::_physics_process(double delta) {
  if (godot::Engine::get_singleton()->is_editor_hint()) {
    return;
  }

  if (!_player || !_player->is_multiplayer_authority()) {
    return;
  }

  _input_state.poll_actions();
  PlayerInputState state = _input_state.consume();

  if (state.primary_action) {
    _player->trigger_right_item_action("primary");
  }

  // toggle
  if (state.toggle_torch) {
    _player->toggle_torch();
  }
  if (state.toggle_picaxe) {
    _player->toggle_picaxe();
  }
  //

  _movement.tick(*_player, state, delta);
  if (_camera_ready) {
    _camera.tick(state, _player->get_sensitivity());
  }
}

} // namespace morphic
