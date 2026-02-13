#include "item_database.h"

#include "utils/bind_methods.h"

namespace morphic {

void ItemDatabase::set_items(Array p_items) {
  _items = p_items;
  rebuild_index();
}

Array ItemDatabase::get_items() const { return _items; }

Ref<ItemDefinition> ItemDatabase::get_item_by_id(int p_id) const {
  const Ref<ItemDefinition> *found = _by_id.getptr(p_id);
  return found ? *found : Ref<ItemDefinition>();
}

int ItemDatabase::get_id_by_name(String p_name) const {
  const int *found = _id_by_name.getptr(StringName(p_name));
  return found ? *found : -1;
}

void ItemDatabase::rebuild_index() {
  _by_id.clear();
  _id_by_name.clear();

  for (int i = 0; i < _items.size(); ++i) {
    Ref<ItemDefinition> def = _items[i];
    if (def.is_null()) {
      continue;
    }

    const int id = def->get_id();
    _by_id[id] = def;

    const String name = def->get_name();
    if (!name.is_empty()) {
      _id_by_name[StringName(name)] = id;
    }
  }
}

void ItemDatabase::_bind_methods() {
  ClassDB::bind_method(D_METHOD("get_item_by_id", "id"),
                       &ItemDatabase::get_item_by_id);
  ClassDB::bind_method(D_METHOD("get_id_by_name", "name"),
                       &ItemDatabase::get_id_by_name);

  ClassDB::bind_method(D_METHOD("set_items", "items"),
                       &ItemDatabase::set_items);
  ClassDB::bind_method(D_METHOD("get_items"), &ItemDatabase::get_items);
  ADD_PROPERTY(PropertyInfo(Variant::ARRAY, "items", PROPERTY_HINT_ARRAY_TYPE,
                            "ItemDefinition"),
               "set_items", "get_items");
}

} // namespace morphic
