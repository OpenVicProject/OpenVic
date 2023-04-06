#include "LoadGoods.hpp"

#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/json.hpp>

using namespace OpenVic2;
using namespace godot;

#define JSON_PROPERTY_NAME(x) godot::StringName(x)

std::vector<Good> LoadGoods::load_goods_from_disk(godot::String const& file_path) {
	std::vector<Good> goods;
	Ref<FileAccess> file = FileAccess::open(file_path, FileAccess::ModeFlags::READ);
	Error err = FileAccess::get_open_error();
	if (err != OK || file.is_null()) {
		UtilityFunctions::push_error("Failed to open good configuration file: ", file_path);
		return std::vector<Good>();
	}
	godot::String file_content = file->get_as_text();
	JSON parsed_goods;
	err = parsed_goods.parse(file_content);
	if (err != OK) {
		UtilityFunctions::push_error("Failed to parse goods.json");
		return std::vector<Good>();
	}
	Variant v_goods = parsed_goods.get_data().get("goods", nullptr);
	if (v_goods.get_type() != Variant::ARRAY) {
		UtilityFunctions::push_error("Failed to parse goods.json: Top level property is not an array");
		return std::vector<Good>();
	}
	godot::Array v_goods_array = (godot::Array)v_goods;
	int32_t count = v_goods_array.size();
	goods.resize(count);
	for (size_t i = 0; i < count; i++) {
		if(!extract_property_from_json(v_goods_array[i], goods, i)) {
			return std::vector<Good>();
		}
	}
	return goods;
}

bool LoadGoods::extract_property_from_json(const godot::Variant& variant, std::vector<Good>& goods, int32_t index) {
	bool valid;
	String id = variant.get_named(JSON_PROPERTY_NAME("id"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property id of type string from goods.json");
		return false;
	}
	String category = variant.get_named(JSON_PROPERTY_NAME("category"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property category of type string from goods.json");
		return false;
	}
	float_t cost = (float_t)variant.get_named(JSON_PROPERTY_NAME("cost"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property cost of type float from goods.json");
		return false;
	}
	String colour = variant.get_named(JSON_PROPERTY_NAME("colour"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property colour of type string from goods.json");
		return false;
	}
	bool isAvailableAtStart = (bool)variant.get_named(JSON_PROPERTY_NAME("isAvailableAtStart"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property isAvailableAtStart of type bool from goods.json");
		return false;
	}
	bool isTradable = (bool)variant.get_named(JSON_PROPERTY_NAME("isTradeable"), valid); 
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property isTradable of type bool from goods.json");
		return false;
	}
	bool isMoney = (bool)variant.get_named(JSON_PROPERTY_NAME("isMoney"), valid); 
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property isMoney of type bool from goods.json");
		return false; 
	}
	bool hasOverseasPenalty = (bool)variant.get_named(JSON_PROPERTY_NAME("hasOverseasPenalty"), valid);
	if(!valid) {
		UtilityFunctions::push_error("Could not extract property hasOverseaPenalty of type bool from goods.json");
		return false;
	}
	goods.at(index) = Good(id, category, cost, colour, isAvailableAtStart, isTradable, isMoney, hasOverseasPenalty);
	return true;
}