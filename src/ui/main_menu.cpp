#include "main_menu.h"
#include "utils/network_utils.h"

#include <godot_cpp/core/class_db.hpp>

using namespace godot;

namespace morphic {

void MainMenu::_ready() {}

void MainMenu::on_host_pressed() {
  NetworkManager *nm = NetUtils::get_net_manager(this);
  if (nm == nullptr)
    return;

  bool result = nm->start_host(7777);
  if (result)
    get_tree()->change_scene_to_file("res://world/world.tscn");
}

void MainMenu::on_join_pressed() {
  NetworkManager *nm = NetUtils::get_net_manager(this);
  if (nm == nullptr)
    return;

  bool result = nm->start_client("localhost", 7777);
  if (result)
    get_tree()->change_scene_to_file("res://world/world.tscn");
}

void MainMenu::_bind_methods() {
  ClassDB::bind_method(D_METHOD("on_host_pressed"), &MainMenu::on_host_pressed);
  ClassDB::bind_method(D_METHOD("on_join_pressed"), &MainMenu::on_join_pressed);
}

} // namespace morphic
