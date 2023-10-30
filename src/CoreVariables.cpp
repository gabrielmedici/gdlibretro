#include "RetroHost.hpp"
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>

godot::String get_variable_file_path(godot::String core_name) {
    if (godot::OS::get_singleton()->has_feature("editor")) {
        return "res://libretro-cores/" + core_name + ".yml";
    } else {
        return "user://libretro-cores/" + core_name + ".yml";
    }
}

void RetroHost::load_core_variables() {
    godot::String path = get_variable_file_path(core_name);

    auto file = godot::FileAccess::open(path, godot::FileAccess::ModeFlags::READ);
    if(godot::FileAccess::get_open_error() != godot::Error::OK) {
        godot::UtilityFunctions::printerr("[RetroHost] Failed (", godot::FileAccess::get_open_error(), ") to open core variables file: ", path);
        this->core_variables = YAML::Node();
        return;
    }

    this->core_variables = YAML::Load(file->get_as_text().utf8().get_data());
    file->close();
    godot::UtilityFunctions::print("[RetroHost] Loaded core variables file: ", path);
}

void RetroHost::save_core_variables() {
    godot::String path = get_variable_file_path(core_name);
    auto file = godot::FileAccess::open(path, godot::FileAccess::ModeFlags::WRITE_READ);
    if(godot::FileAccess::get_open_error() != godot::Error::OK) {
        godot::UtilityFunctions::printerr("[RetroHost] Failed (", godot::FileAccess::get_open_error(), ") to save core variables file: ", path);
        return;
    }

    godot::UtilityFunctions::print("[RetroHost] Saving core variables file");

    file->store_string(YAML::Dump(this->core_variables).c_str());
    file->close();
}