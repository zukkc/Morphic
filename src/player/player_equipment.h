#pragma once

#include <godot_cpp/classes/node.hpp>
#include <godot_cpp/variant/string.hpp>

#include "items/item_database.h"

using namespace godot;

namespace morphic {

class PlayerEquipment : public Node {
  GDCLASS(PlayerEquipment, Node)

public:
  enum Hand { LEFT, RIGHT };

protected:
  static void _bind_methods();

public:
  PlayerEquipment() = default;
  ~PlayerEquipment() = default;

  void equip_left(int item_id);
  void equip_right(int item_id);
  void equip_left_by_name(String item_name);
  void equip_right_by_name(String item_name);
  void clear_left();
  void clear_right();

  int get_item_id_by_name(String p_name) const;
  Ref<ItemDefinition> get_left_item_def() const;
  Ref<ItemDefinition> get_right_item_def() const;

  void set_left_item(int p_item);
  int get_left_item() const;

  void set_right_item(int p_item);
  int get_right_item() const;

  void set_item_database(const Ref<ItemDatabase> &p_item_database);
  Ref<ItemDatabase> get_item_database() const;

private:
  int _left_item = 0;
  int _right_item = 0;
  Ref<ItemDatabase> _item_database;
};

} // namespace morphic

VARIANT_ENUM_CAST(morphic::PlayerEquipment::Hand);
