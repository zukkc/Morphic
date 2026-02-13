#include "player_equipment.h"

#include "utils/bind_methods.h"
#include <godot_cpp/core/error_macros.hpp>

namespace morphic {

void PlayerEquipment::equip_left(int item_id) { set_left_item(item_id); }
void PlayerEquipment::equip_right(int item_id) { set_right_item(item_id); }

void PlayerEquipment::equip_left_by_name(String item_name) {
  const int id = get_item_id_by_name(item_name);
  if (id <= 0) {
    ERR_PRINT("PlayerEquipment: Unknown item name (left).");
    return;
  }
  set_left_item(id);
}

void PlayerEquipment::equip_right_by_name(String item_name) {
  const int id = get_item_id_by_name(item_name);
  if (id <= 0) {
    ERR_PRINT("PlayerEquipment: Unknown item name (right).");
    return;
  }
  set_right_item(id);
}

void PlayerEquipment::clear_left() { set_left_item(0); }
void PlayerEquipment::clear_right() { set_right_item(0); }

void PlayerEquipment::set_left_item(int p_item) {
  Ref<ItemDefinition> def;
  if (p_item != 0) {
    if (_item_database.is_null()) {
      ERR_PRINT("PlayerEquipment: ItemDatabase not set (left).");
      return;
    }
    def = _item_database->get_item_by_id(p_item);
    if (def.is_null()) {
      ERR_PRINT("PlayerEquipment: Unknown item id (left).");
      return;
    }
  }
  if (_left_item == p_item) {
    return;
  }
  _left_item = p_item;
  emit_signal("left_item_equipped", def);
}
int PlayerEquipment::get_left_item() const { return _left_item; }

void PlayerEquipment::set_right_item(int p_item) {
  Ref<ItemDefinition> def;
  if (p_item != 0) {
    if (_item_database.is_null()) {
      ERR_PRINT("PlayerEquipment: ItemDatabase not set (right).");
      return;
    }
    def = _item_database->get_item_by_id(p_item);
    if (def.is_null()) {
      ERR_PRINT("PlayerEquipment: Unknown item id (right).");
      return;
    }
  }
  if (_right_item == p_item) {
    return;
  }
  _right_item = p_item;
  emit_signal("right_item_equipped", def);
}
int PlayerEquipment::get_right_item() const { return _right_item; }

int PlayerEquipment::get_item_id_by_name(String p_name) const {
  if (_item_database.is_null()) {
    return -1;
  }
  return _item_database->get_id_by_name(p_name);
}

Ref<ItemDefinition> PlayerEquipment::get_left_item_def() const {
  if (_item_database.is_null()) {
    return Ref<ItemDefinition>();
  }
  if (_left_item == 0) {
    return Ref<ItemDefinition>();
  }
  return _item_database->get_item_by_id(_left_item);
}

Ref<ItemDefinition> PlayerEquipment::get_right_item_def() const {
  if (_item_database.is_null()) {
    return Ref<ItemDefinition>();
  }
  if (_right_item == 0) {
    return Ref<ItemDefinition>();
  }
  return _item_database->get_item_by_id(_right_item);
}

void PlayerEquipment::set_item_database(
    const Ref<ItemDatabase> &p_item_database) {
  _item_database = p_item_database;
}
Ref<ItemDatabase> PlayerEquipment::get_item_database() const {
  return _item_database;
}

void PlayerEquipment::_bind_methods() {
  ClassDB::bind_method(D_METHOD("equip_left", "item_id"),
                       &PlayerEquipment::equip_left);
  ClassDB::bind_method(D_METHOD("equip_right", "item_id"),
                       &PlayerEquipment::equip_right);
  ClassDB::bind_method(D_METHOD("equip_left_by_name", "item_name"),
                       &PlayerEquipment::equip_left_by_name);
  ClassDB::bind_method(D_METHOD("equip_right_by_name", "item_name"),
                       &PlayerEquipment::equip_right_by_name);
  ClassDB::bind_method(D_METHOD("clear_left"), &PlayerEquipment::clear_left);
  ClassDB::bind_method(D_METHOD("clear_right"), &PlayerEquipment::clear_right);
  ClassDB::bind_method(D_METHOD("get_item_id_by_name", "name"),
                       &PlayerEquipment::get_item_id_by_name);
  ClassDB::bind_method(D_METHOD("get_left_item_def"),
                       &PlayerEquipment::get_left_item_def);
  ClassDB::bind_method(D_METHOD("get_right_item_def"),
                       &PlayerEquipment::get_right_item_def);

  BIND_PROPERTY(PlayerEquipment, Variant::INT, "left_item", left_item);
  BIND_PROPERTY(PlayerEquipment, Variant::INT, "right_item", right_item);
  BIND_PROPERTY_HINT(PlayerEquipment, Variant::OBJECT, "item_database",
                     item_database, PROPERTY_HINT_RESOURCE_TYPE);

  ADD_SIGNAL(MethodInfo("left_item_equipped",
                        PropertyInfo(Variant::OBJECT, "item",
                                     PROPERTY_HINT_RESOURCE_TYPE,
                                     "ItemDefinition")));
  ADD_SIGNAL(MethodInfo("right_item_equipped",
                        PropertyInfo(Variant::OBJECT, "item",
                                     PROPERTY_HINT_RESOURCE_TYPE,
                                     "ItemDefinition")));

  BIND_ENUM_CONSTANT(LEFT);
  BIND_ENUM_CONSTANT(RIGHT);
}

} // namespace morphic
