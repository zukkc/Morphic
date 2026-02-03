#pragma once

namespace morphic {

struct PlayerInputState;
class Player;

class PlayerMovement {
public:
  void tick(Player &player, const PlayerInputState &input, double delta);
};

} // namespace morphic
