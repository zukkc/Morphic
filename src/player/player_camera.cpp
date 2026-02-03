#include "player_camera.h"

#include "player_input.h"
#include "utils/debug_utils.h"

#include <godot_cpp/core/math.hpp>

namespace morphic {

bool PlayerCamera::setup(godot::Node3D *body, godot::Node3D *head) {
  _body = body;
  _head = head;

  if (!_head) {
    ERR_PRINT("PlayerCamera: Head node not found");
    return false;
  }

  godot::Node *camera_node = _head->find_child("Camera3D");
  _camera = godot::Object::cast_to<godot::Camera3D>(camera_node);

  if (!_camera) {
    ERR_PRINT("Player camera not found inside Head!");
    return false;
  }

  _camera->set_current(true);
  return true;
}

void PlayerCamera::tick(const PlayerInputState &input, float sensitivity) {
  if (!_body || !_head) {
    return;
  }

  if (input.look == godot::Vector2()) {
    return;
  }

  _body->rotate_y(-input.look.x * sensitivity);
  _head->rotate_x(-input.look.y * sensitivity);

  godot::Vector3 rot = _head->get_rotation();
  rot.x = godot::Math::clamp(rot.x, godot::Math::deg_to_rad(-89.0f),
                             godot::Math::deg_to_rad(89.0f));
  _head->set_rotation(rot);
}

} // namespace morphic
