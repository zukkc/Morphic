#pragma once

#include "godot_cpp/classes/camera3d.hpp"
#include "godot_cpp/classes/node3d.hpp"

namespace morphic {

struct PlayerInputState;

class PlayerCamera {
public:
  bool setup(godot::Node3D *body, godot::Node3D *head);
  void tick(const PlayerInputState &input, float sensitivity);

private:
  godot::Node3D *_body = nullptr;
  godot::Node3D *_head = nullptr;
  godot::Camera3D *_camera = nullptr;
};

} // namespace morphic
