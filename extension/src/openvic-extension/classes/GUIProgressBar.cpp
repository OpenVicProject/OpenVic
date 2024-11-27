#include "GUIProgressBar.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include <openvic-extension/singletons/AssetManager.hpp>
#include <openvic-extension/utility/Utilities.hpp>

using namespace godot;
using namespace OpenVic;

GUI_TOOLTIP_IMPLEMENTATIONS(GUIProgressBar)

void GUIProgressBar::_bind_methods() {
	GUI_TOOLTIP_BIND_METHODS(GUIProgressBar)
}

void GUIProgressBar::_notification(int what) {
	_tooltip_notification(what);
}

GUIProgressBar::GUIProgressBar() : tooltip_active { false } {}

Error GUIProgressBar::set_gfx_progress_bar(GFX::ProgressBar const* progress_bar) {
	ERR_FAIL_NULL_V(progress_bar, FAILED);

	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, FAILED);

	Error err = OK;

	static constexpr double MIN_VALUE = 0.0, MAX_VALUE = 1.0;
	static constexpr uint32_t STEPS = 100;

	set_nine_patch_stretch(true);
	set_step((MAX_VALUE - MIN_VALUE) / STEPS);
	set_min(MIN_VALUE);
	set_max(MAX_VALUE);

	using enum AssetManager::LoadFlags;

	Ref<ImageTexture> back_texture;
	if (!progress_bar->get_back_texture_file().empty()) {
		const StringName back_texture_file = Utilities::std_to_godot_string(progress_bar->get_back_texture_file());
		back_texture = asset_manager->get_texture(back_texture_file, LOAD_FLAG_CACHE_TEXTURE | LOAD_FLAG_FLIP_Y);
		if (back_texture.is_null()) {
			UtilityFunctions::push_error(
				"Failed to load sprite back texture ", back_texture_file, " for GUIProgressBar ", get_name()
			);
			err = FAILED;
		}
	}
	if (back_texture.is_null()) {
		const Color back_colour = Utilities::to_godot_color(progress_bar->get_back_colour());
		back_texture =
			Utilities::make_solid_colour_texture(back_colour, progress_bar->get_size().x, progress_bar->get_size().y);
		if (back_texture.is_null()) {
			UtilityFunctions::push_error(
				"Failed to generate sprite ", back_colour, " back texture for GUIProgressBar ", get_name()
			);
			err = FAILED;
		}
	}
	if (back_texture.is_valid()) {
		set_under_texture(back_texture);
	} else {
		UtilityFunctions::push_error("Failed to create and set sprite back texture for GUIProgressBar ", get_name());
		err = FAILED;
	}

	Ref<ImageTexture> progress_texture;
	if (!progress_bar->get_progress_texture_file().empty()) {
		const StringName progress_texture_file = Utilities::std_to_godot_string(progress_bar->get_progress_texture_file());
		progress_texture = asset_manager->get_texture(progress_texture_file, LOAD_FLAG_CACHE_TEXTURE | LOAD_FLAG_FLIP_Y);
		if (progress_texture.is_null()) {
			UtilityFunctions::push_error(
				"Failed to load sprite progress texture ", progress_texture_file, " for GUIProgressBar ", get_name()
			);
			err = FAILED;
		}
	}
	if (progress_texture.is_null()) {
		const Color progress_colour = Utilities::to_godot_color(progress_bar->get_progress_colour());
		progress_texture =
			Utilities::make_solid_colour_texture(progress_colour, progress_bar->get_size().x, progress_bar->get_size().y);
		if (progress_texture.is_null()) {
			UtilityFunctions::push_error(
				"Failed to generate sprite ", progress_colour, " progress texture for GUIProgressBar ", get_name()
			);
			err = FAILED;
		}
	}
	if (progress_texture.is_valid()) {
		set_progress_texture(progress_texture);
	} else {
		UtilityFunctions::push_error("Failed to create and set sprite progress texture for GUIProgressBar ", get_name());
		err = FAILED;
	}

	// TODO - work out why progress bar is missing bottom border pixel (e.g. province building expansion bar)
	set_custom_minimum_size(Utilities::to_godot_fvec2(static_cast<fvec2_t>(progress_bar->get_size())));

	return err;
}
