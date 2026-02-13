#include "item_action.h"

#include "utils/bind_methods.h"

namespace morphic {

void ItemAction::set_action(String p_action) { _action = p_action; }
String ItemAction::get_action() const { return _action; }

void ItemAction::set_hand_state(String p_state) { _hand_state = p_state; }
String ItemAction::get_hand_state() const { return _hand_state; }

void ItemAction::set_full_body_state(String p_state) {
  _full_body_state = p_state;
}
String ItemAction::get_full_body_state() const { return _full_body_state; }

void ItemAction::_bind_methods() {
  BIND_PROPERTY(ItemAction, Variant::STRING, "action", action);
  BIND_PROPERTY(ItemAction, Variant::STRING, "hand_state", hand_state);
  BIND_PROPERTY(ItemAction, Variant::STRING, "full_body_state",
                full_body_state);
}

} // namespace morphic
