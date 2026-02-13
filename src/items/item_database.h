#pragma once

#include "item_definition.h"

#include <godot_cpp/classes/resource.hpp>
#include <godot_cpp/templates/hash_map.hpp>
#include <godot_cpp/variant/array.hpp>
#include <godot_cpp/variant/string.hpp>
#include <godot_cpp/variant/string_name.hpp>

using namespace godot;

namespace morphic {

class ItemDatabase : public Resource {
  GDCLASS(ItemDatabase, Resource)

protected:
  static void _bind_methods();

public:
  void set_items(Array p_items);
  Array get_items() const;

  Ref<ItemDefinition> get_item_by_id(int p_id) const;
  int get_id_by_name(String p_name) const;

private:
  Array _items;
  HashMap<int, Ref<ItemDefinition>> _by_id;
  HashMap<StringName, int> _id_by_name;

  void rebuild_index();
};

} // namespace morphic
