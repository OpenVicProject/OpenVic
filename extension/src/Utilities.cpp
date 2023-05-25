#include "Utilities.hpp"

#include <godot_cpp/classes/resource_loader.hpp>
#include <godot_cpp/variant/utility_functions.hpp>

using namespace godot;
using namespace OpenVic;

Ref<Image> OpenVic::load_godot_image(String const& path) {
	if (path.begins_with("res://")) {
		ResourceLoader* loader = ResourceLoader::get_singleton();
		return loader ? loader->load(path) : nullptr;
	} else {
		return Image::load_from_file(path);
	}
}
