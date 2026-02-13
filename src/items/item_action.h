#pragma once

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/variant/string.hpp>

using namespace godot;

namespace morphic {

class ItemAction : public Resource {
  GDCLASS(ItemAction, Resource)

protected:
  static void _bind_methods();

public:
  void set_action(String p_action);
  String get_action() const;

  void set_hand_state(String p_state);
  String get_hand_state() const;

  void set_full_body_state(String p_state);
  String get_full_body_state() const;

private:
  String _action;
  String _hand_state;
  String _full_body_state;
};

} // namespace morphic
