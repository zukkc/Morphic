#pragma once
#include <godot_cpp/classes/control.hpp>

namespace morphic {

class MainMenu : public godot::Control {
  GDCLASS(MainMenu, godot::Control)

protected:
  static void _bind_methods();

public:
  void _ready() override;

  void on_host_pressed();
  void on_join_pressed();
};

} // namespace morphic
