#include "item_definition.h"
#include "item_action.h"

#include "utils/bind_methods.h"

namespace morphic {

void ItemDefinition::set_id(int p_id) { _id = p_id; }
int ItemDefinition::get_id() const { return _id; }

void ItemDefinition::set_name(String p_name) { _name = p_name; }
String ItemDefinition::get_name() const { return _name; }

void ItemDefinition::set_hand(int p_hand) { _hand = p_hand; }
int ItemDefinition::get_hand() const { return _hand; }

void ItemDefinition::set_equip_state(String p_state) { _equip_state = p_state; }
String ItemDefinition::get_equip_state() const { return _equip_state; }

void ItemDefinition::set_model_scene(const Ref<PackedScene> &p_scene) {
  _model_scene = p_scene;
}
Ref<PackedScene> ItemDefinition::get_model_scene() const {
  return _model_scene;
}

void ItemDefinition::set_hand_playback(String p_name) {
  _hand_playback = p_name;
}
String ItemDefinition::get_hand_playback() const { return _hand_playback; }

void ItemDefinition::set_actions(Array p_actions) { _actions = p_actions; }
Array ItemDefinition::get_actions() const { return _actions; }

static Ref<ItemAction> find_action(const Array &actions, String action) {
  for (int i = 0; i < actions.size(); ++i) {
    Ref<ItemAction> entry = actions[i];
    if (entry.is_valid() && entry->get_action() == action) {
      return entry;
    }
  }
  return Ref<ItemAction>();
}

void ItemDefinition::set_hand_action(String action, String state) {
  Ref<ItemAction> entry = find_action(_actions, action);
  if (entry.is_null()) {
    entry.instantiate();
    entry->set_action(action);
    _actions.append(entry);
  }
  entry->set_hand_state(state);
}

String ItemDefinition::get_hand_action(String action) const {
  Ref<ItemAction> entry = find_action(_actions, action);
  if (entry.is_valid()) {
    return entry->get_hand_state();
  }
  return String();
}

void ItemDefinition::set_full_body_action(String action, String state) {
  Ref<ItemAction> entry = find_action(_actions, action);
  if (entry.is_null()) {
    entry.instantiate();
    entry->set_action(action);
    _actions.append(entry);
  }
  entry->set_full_body_state(state);
}

String ItemDefinition::get_full_body_action(String action) const {
  Ref<ItemAction> entry = find_action(_actions, action);
  if (entry.is_valid()) {
    return entry->get_full_body_state();
  }
  return String();
}

void ItemDefinition::_bind_methods() {
  BIND_PROPERTY(ItemDefinition, Variant::INT, "id", id);
  BIND_PROPERTY(ItemDefinition, Variant::STRING, "name", name);
  BIND_PROPERTY(ItemDefinition, Variant::INT, "hand", hand);
  BIND_PROPERTY(ItemDefinition, Variant::STRING, "equip_state", equip_state);
  BIND_PROPERTY_HINT(ItemDefinition, Variant::OBJECT, "model_scene",
                     model_scene, PROPERTY_HINT_RESOURCE_TYPE);
  BIND_PROPERTY(ItemDefinition, Variant::STRING, "hand_playback",
                hand_playback);

  ClassDB::bind_method(D_METHOD("set_actions", "actions"),
                       &ItemDefinition::set_actions);
  ClassDB::bind_method(D_METHOD("get_actions"), &ItemDefinition::get_actions);
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "actions", PROPERTY_HINT_ARRAY_TYPE,
                            "ItemAction"),
               "set_actions", "get_actions");

  ClassDB::bind_method(D_METHOD("set_hand_action", "action", "state"),
                       &ItemDefinition::set_hand_action);
  ClassDB::bind_method(D_METHOD("get_hand_action", "action"),
                       &ItemDefinition::get_hand_action);
  ClassDB::bind_method(D_METHOD("set_full_body_action", "action", "state"),
                       &ItemDefinition::set_full_body_action);
  ClassDB::bind_method(D_METHOD("get_full_body_action", "action"),
                       &ItemDefinition::get_full_body_action);

  BIND_ENUM_CONSTANT(LEFT);
  BIND_ENUM_CONSTANT(RIGHT);
  BIND_ENUM_CONSTANT(BOTH);
}

} // namespace morphic
