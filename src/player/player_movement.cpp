#include "player_movement.h"

#include "player.h"
#include "player_input.h"

#include <godot_cpp/core/math.hpp>

namespace morphic {

void PlayerMovement::tick(Player &player, const PlayerInputState &input,
                          double delta) {
  Vector3 velocity = player.get_velocity();

  if (!player.is_on_floor()) {
    velocity.y -= player.get_gravity() * delta;
  }

  if (input.jump && player.is_on_floor()) {
    velocity.y = player.get_jump_velocity();
  }

  Vector3 direction =
      player.get_transform()
          .basis.xform(Vector3(input.move.x, 0, input.move.y))
          .normalized();

  if (direction != Vector3(0, 0, 0)) {
    const double speed = player.get_speed();
    velocity.x = direction.x * speed;
    velocity.z = direction.z * speed;
  } else {
    const double friction = player.get_friction();
    const double speed = player.get_speed();
    velocity.x = Math::move_toward(velocity.x, 0,
                                   (real_t)(speed * friction * delta));
    velocity.z = Math::move_toward(velocity.z, 0,
                                   (real_t)(speed * friction * delta));
  }

  player.set_velocity(velocity);
  player.move_and_slide();
}

} // namespace morphic
