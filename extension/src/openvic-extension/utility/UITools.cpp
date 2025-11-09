#include "UITools.hpp"

#include <cctype>

#include <godot_cpp/classes/audio_server.hpp>
#include <godot_cpp/classes/audio_stream_player.hpp>
#include <godot_cpp/classes/audio_stream_wav.hpp>
#include <godot_cpp/classes/base_button.hpp>
#include <godot_cpp/classes/color_rect.hpp>
#include <godot_cpp/classes/global_constants.hpp>
#include <godot_cpp/classes/input_event_action.hpp>
#include <godot_cpp/classes/input_event_key.hpp>
#include <godot_cpp/classes/input_map.hpp>
#include <godot_cpp/classes/line_edit.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/panel.hpp>
#include <godot_cpp/classes/scene_tree.hpp>
#include <godot_cpp/classes/shortcut.hpp>
#include <godot_cpp/classes/style_box_empty.hpp>
#include <godot_cpp/classes/style_box_texture.hpp>
#include <godot_cpp/classes/theme.hpp>
#include <godot_cpp/classes/window.hpp>
#include <godot_cpp/core/error_macros.hpp>
#include <godot_cpp/variant/callable_method_pointer.hpp>
#include <godot_cpp/variant/packed_byte_array.hpp>
#include <godot_cpp/variant/utility_functions.hpp>
#include <godot_cpp/variant/variant.hpp>

#include <openvic-simulation/utility/Containers.hpp>

#include "openvic-extension/classes/GUIButton.hpp"
#include "openvic-extension/classes/GUIIcon.hpp"
#include "openvic-extension/classes/GUIIconButton.hpp"
#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUILineChart.hpp"
#include "openvic-extension/classes/GUIListBox.hpp"
#include "openvic-extension/classes/GUIMaskedFlag.hpp"
#include "openvic-extension/classes/GUIMaskedFlagButton.hpp"
#include "openvic-extension/classes/GUIOverlappingElementsBox.hpp"
#include "openvic-extension/classes/GUIPieChart.hpp"
#include "openvic-extension/classes/GUIProgressBar.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/AssetManager.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/SoundSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

#include "openvic-simulation/misc/SoundEffect.hpp"

using namespace godot;
using namespace OpenVic;

GFX::Sprite const* UITools::get_gfx_sprite(String const& gfx_sprite) {
	GameSingleton* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GFX::Sprite const* sprite = game_singleton->get_definition_manager().get_ui_manager().get_sprite_by_identifier(
		Utilities::godot_to_std_string(gfx_sprite)
	);
	ERR_FAIL_NULL_V_MSG(sprite, nullptr, Utilities::format("GFX sprite not found: %s", gfx_sprite));
	return sprite;
}

GUI::Element const* UITools::get_gui_element(String const& gui_scene, String const& gui_element) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene = game_singleton->get_definition_manager().get_ui_manager().get_scene_by_identifier(
		Utilities::godot_to_std_string(gui_scene)
	);
	ERR_FAIL_NULL_V_MSG(scene, nullptr, Utilities::format("Failed to find GUI scene %s", gui_scene));
	GUI::Element const* element = scene->get_scene_element_by_identifier(Utilities::godot_to_std_string(gui_element));
	ERR_FAIL_NULL_V_MSG(element, nullptr, Utilities::format("Failed to find GUI element %s in GUI scene %s", gui_element, gui_scene));
	return element;
}

GUI::Position const* UITools::get_gui_position(String const& gui_scene, String const& gui_position) {
	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	ERR_FAIL_NULL_V(game_singleton, nullptr);
	GUI::Scene const* scene = game_singleton->get_definition_manager().get_ui_manager().get_scene_by_identifier(
		Utilities::godot_to_std_string(gui_scene)
	);
	ERR_FAIL_NULL_V_MSG(scene, nullptr, Utilities::format("Failed to find GUI scene %s", gui_scene));
	GUI::Position const* position = scene->get_scene_position_by_identifier(Utilities::godot_to_std_string(gui_position));
	ERR_FAIL_NULL_V_MSG(position, nullptr, Utilities::format("Failed to find GUI position %s in GUI scene %s", gui_position, gui_scene));
	return position;
}

static Array get_events_from_shortcut_key(String const& key) {
	Array events;
	if (key.is_empty()) {
		return events;
	}

	Key key_value = OS::get_singleton()->find_keycode_from_string(key);
	if (key_value == Key::KEY_UNKNOWN) {
		if (key.nocasecmp_to("DEL") == 0) {
			key_value = Key::KEY_DELETE;
		} else if (key.nocasecmp_to("PAGE_UP") == 0) {
			key_value = Key::KEY_PAGEUP;
		} else if (key.nocasecmp_to("PAGE_DOWN") == 0) {
			key_value = Key::KEY_PAGEDOWN;
		} else if (key == "+") {
			// on most keyboards + and = are on the same key, Godot does not see them as different
			key_value = Key::KEY_EQUAL;
		} else if (key == "-") {
			key_value = Key::KEY_MINUS;
		} else if (key == ">") {
			// on many keyboards > and . are on the same key, Godot does not see them as different
			key_value = Key::KEY_PERIOD;
		} else if (key == "<") {
			// on many keyboards < and , are on the same key, Godot does not see them as different
			key_value = Key::KEY_COMMA;
		}
	}

	if (key_value == Key::KEY_UNKNOWN) {
		return events;
	}

	Ref<InputEventKey> event;
	event.instantiate();
	event->set_pressed(true);
	events.append(event);

	if (key.length() == 1) {
		if (key_value >= Key::KEY_A && key_value <= Key::KEY_Z) {
			event->set_shift_pressed(std::isupper(key[0]));
		} else if (key == "+") {
			event->set_shift_pressed(true);

			Ref<InputEventKey> second_event;
			second_event.instantiate();
			second_event->set_pressed(true);
			second_event->set_key_label(godot::KEY_KP_ADD);
			events.append(second_event);
		} else if (key == ">") {
			event->set_shift_pressed(true);
		}
	}

	event->set_key_label(key_value);
	return events;
}

static Error try_create_shortcut_action_for_button(
	GUIButton* gui_button, String const& shortcut_key_name, String const& shortcut_hotkey_name = ""
) {
	if (shortcut_key_name.is_empty()) {
		return OK;
	}

	Array event_array = get_events_from_shortcut_key(shortcut_key_name);

	ERR_FAIL_COND_V_MSG(
		event_array.is_empty(), ERR_INVALID_PARAMETER,
		Utilities::format("Unknown shortcut key '%s' for GUI button %s", shortcut_key_name, gui_button->get_name())
	);

	InputMap* const im = InputMap::get_singleton();
	String action_name;
	if (shortcut_hotkey_name.is_empty()) {
		action_name = //
			Utilities::format("button_%s_hotkey", gui_button->get_name().to_lower().replace("button", "").replace("hotkey", ""))
				.replace("__", "_");
	} else {
		action_name = Utilities::format("button_%s_hotkey", shortcut_hotkey_name);
	}
	Ref<InputEventAction> action_event;
	action_event.instantiate();
	action_event->set_action(action_name);
	action_event->set_pressed(true);

	if (im->has_action(action_name)) {
		TypedArray<InputEvent> events = im->action_get_events(action_name);
		bool should_warn = events.size() != event_array.size();
		if (!should_warn) {
			for (std::size_t index = 0; index < events.size(); index++) {
				if (!event_array.has(events[index])) {
					should_warn = true;
					break;
				}
			}
		}

		if (should_warn) {
			WARN_PRINT(Utilities::format("'%s' already found in InputMap with different values, reusing hotkey", action_name));
		}
	} else {
		im->add_action(action_name);
		for (std::size_t index = 0; index < event_array.size(); index++) {
			Ref<InputEvent> event = event_array[index];
			ERR_CONTINUE(event.is_null());
			im->action_add_event(action_name, event);
		}
	}

	Array shortcut_array;
	shortcut_array.push_back(action_event);

	Ref<Shortcut> shortcut;
	shortcut.instantiate();
	shortcut->set_events(shortcut_array);
	gui_button->set_shortcut(shortcut);

	return OK;
}

/* GUI::Element tree -> godot::Control tree conversion code below: */

namespace OpenVic {
	struct generate_gui_args_t {
		GUI::Element const& element;
		String const& name;
		AssetManager& asset_manager;
		Control*& result;

		constexpr generate_gui_args_t(
			GUI::Element const& new_element, String const& new_name, AssetManager& new_asset_manager, Control*& new_result
		) : element { new_element }, name { new_name }, asset_manager { new_asset_manager }, result { new_result } {}
	};
}

template<std::derived_from<Control> T>
static bool new_control(T*& node, GUI::Element const& element, String const& name) {
	node = memnew(T);
	ERR_FAIL_NULL_V(node, false);

	using enum GUI::Element::orientation_t;
	using enum Control::LayoutPreset;
	static const ordered_map<GUI::Element::orientation_t, Control::LayoutPreset> orientation_map {
		{ UPPER_LEFT, PRESET_TOP_LEFT }, { LOWER_LEFT, PRESET_BOTTOM_LEFT },
		{ LOWER_RIGHT, PRESET_BOTTOM_RIGHT }, { UPPER_RIGHT, PRESET_TOP_RIGHT },
		{ CENTER, PRESET_CENTER }
	};

	if (name.is_empty()) {
		node->set_name(Utilities::std_to_godot_string(element.get_name()));
	} else {
		node->set_name(name);
	}

	bool ret = true;
	const decltype(orientation_map)::const_iterator it = orientation_map.find(element.get_orientation());
	if (it != orientation_map.end()) {
		node->set_anchors_and_offsets_preset(it->second);
	} else {
		UtilityFunctions::push_error(
			"Invalid orientation for GUI element ", Utilities::std_to_godot_string(element.get_name())
		);
		ret = false;
	}

	node->set_position(Utilities::to_godot_fvec2(element.get_position()));
	node->set_h_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_v_size_flags(Control::SizeFlags::SIZE_SHRINK_BEGIN);
	node->set_focus_mode(Control::FOCUS_NONE);

	return ret;
}

static bool generate_icon(generate_gui_args_t&& args) {
	using namespace OpenVic::Utilities::literals;

	GUI::Icon const& icon = static_cast<GUI::Icon const&>(args.element);

	const String icon_name = Utilities::std_to_godot_string(icon.get_name());

	/* Change to use sprite type to choose Godot node type! */
	bool ret = true;
	if (icon.get_sprite() != nullptr) {
		if (GFX::IconTextureSprite const* texture_sprite = icon.get_sprite()->cast_to<GFX::IconTextureSprite>()) {
			GUIIcon* gui_icon = nullptr;
			ret &= new_control(gui_icon, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				gui_icon, false, Utilities::format("Failed to create GUIIcon for GUI icon %s", icon_name)
			);

			gui_icon->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

			if (gui_icon->set_gfx_texture_sprite(texture_sprite, icon.get_frame()) != OK) {
				UtilityFunctions::push_error("Error setting up GUIIcon for GUI icon ", icon_name);
				ret = false;
			}

			const float scale = static_cast<float>(icon.get_scale());
			gui_icon->set_scale({ scale, scale });

			args.result = gui_icon;
		} else if (GFX::MaskedFlag const* masked_flag = icon.get_sprite()->cast_to<GFX::MaskedFlag>()) {
			GUIMaskedFlag* gui_masked_flag = nullptr;
			ret &= new_control(gui_masked_flag, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				gui_masked_flag, false, Utilities::format("Failed to create GUIMaskedFlag for GUI icon %s", icon_name)
			);

			if (gui_masked_flag->set_gfx_masked_flag(masked_flag) != OK) {
				UtilityFunctions::push_error("Error setting up GUIMaskedFlag for GUI icon ", icon_name);
				ret = false;
			}

			args.result = gui_masked_flag;
		} else if (GFX::ProgressBar const* progress_bar = icon.get_sprite()->cast_to<GFX::ProgressBar>()) {
			GUIProgressBar* gui_progress_bar = nullptr;
			ret &= new_control(gui_progress_bar, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				gui_progress_bar, false, Utilities::format("Failed to create GUIProgressBar for GUI icon %s", icon_name)
			);

			if (gui_progress_bar->set_gfx_progress_bar(progress_bar) != OK) {
				UtilityFunctions::push_error("Error setting up GUIProgressBar for GUI icon ", icon_name);
				ret = false;
			}

			args.result = gui_progress_bar;
		} else if (GFX::PieChart const* pie_chart = icon.get_sprite()->cast_to<GFX::PieChart>()) {
			GUIPieChart* gui_pie_chart = nullptr;
			ret &= new_control(gui_pie_chart, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				gui_pie_chart, false, Utilities::format("Failed to create GUIPieChart for GUI icon %s", icon_name)
			);

			if (gui_pie_chart->set_gfx_pie_chart(pie_chart) == OK) {
				// For some reason pie charts are defined by their top-centre position, so we need to subtract
				// half the width from the x-coordinate to fix this and position the GUIPieChart correctly.
				Vector2 pos = gui_pie_chart->get_position();
				pos.x -= gui_pie_chart->get_gfx_pie_chart_texture()->get_width() / 2.0_real;
				gui_pie_chart->set_position(pos);
			} else {
				UtilityFunctions::push_error("Error setting up GUIPieChart for GUI icon ", icon_name);
				ret = false;
			}

			args.result = gui_pie_chart;
		} else if (GFX::LineChart const* line_chart = icon.get_sprite()->cast_to<GFX::LineChart>()) {
			GUILineChart* gui_line_chart = nullptr;
			ret &= new_control(gui_line_chart, icon, args.name);
			ERR_FAIL_NULL_V_MSG(
				gui_line_chart, false, Utilities::format("Failed to create GUILineChart for GUI icon %s", icon_name)
			);

			if (gui_line_chart->set_gfx_line_chart(line_chart) != OK) {
				UtilityFunctions::push_error("Error setting up GUILineChart for GUI icon ", icon_name);
				ret = false;
			}

			args.result = gui_line_chart;
		} else {
			UtilityFunctions::push_error(
				"Invalid sprite type ", Utilities::std_to_godot_string(icon.get_sprite()->get_type()),
				" for GUI icon ", icon_name
			);
			ret = false;
		}

		if (args.result != nullptr) {
			const real_t rotation = static_cast<real_t>(icon.get_rotation());
			if (rotation != 0.0_real) {
				args.result->set_position(
					args.result->get_position() - args.result->get_custom_minimum_size().height * Vector2 {
						Math::sin(rotation), Math::cos(rotation) - 1.0_real
					}
				);
				args.result->set_rotation(-rotation);
			}
		}
	} else {
		UtilityFunctions::push_error("Null sprite for GUI icon ", icon_name);
		ret = false;
	}
	return ret;
}

static bool generate_button(generate_gui_args_t&& args) {
	GUI::Button const& button = static_cast<GUI::Button const&>(args.element);

	// TODO - rotation (?)
	const String button_name = Utilities::std_to_godot_string(button.get_name());
	const String shortcut_key_name = Utilities::std_to_godot_string(button.get_shortcut());
	SoundEffect const* clicksound = button.get_clicksound();

	ERR_FAIL_NULL_V_MSG(button.get_sprite(), false, Utilities::format("Null sprite for GUI button %s", button_name));

	GUIButton* gui_button = nullptr;
	bool ret = true;

	if (GFX::IconTextureSprite const* texture_sprite = button.get_sprite()->cast_to<GFX::IconTextureSprite>()) {
		GUIIconButton* gui_icon_button = nullptr;
		ret &= new_control(gui_icon_button, button, args.name);
		ERR_FAIL_NULL_V_MSG(gui_icon_button, false, Utilities::format("Failed to create GUIIconButton for GUI button %s", button_name));

		if (gui_icon_button->set_gfx_texture_sprite(texture_sprite) != OK) {
			UtilityFunctions::push_error("Error setting up GUIIconButton for GUI button ", button_name);
			ret = false;
		}

		gui_button = gui_icon_button;
	} else if (GFX::MaskedFlag const* masked_flag = button.get_sprite()->cast_to<GFX::MaskedFlag>()) {
		GUIMaskedFlagButton* gui_masked_flag_button = nullptr;
		ret &= new_control(gui_masked_flag_button, button, args.name);
		ERR_FAIL_NULL_V_MSG(
			gui_masked_flag_button, false, Utilities::format("Failed to create GUIMaskedFlagButton for GUI button %s", button_name)
		);

		if (gui_masked_flag_button->set_gfx_masked_flag(masked_flag) != OK) {
			UtilityFunctions::push_error("Error setting up GUIMaskedFlagButton for GUI button ", button_name);
			ret = false;
		}

		gui_button = gui_masked_flag_button;
	} else {
		ERR_FAIL_V_MSG(
			false, Utilities::format(
				"Invalid sprite type %s for GUI button %s", Utilities::std_to_godot_string(button.get_sprite()->get_type()),
				button_name
			)
		);
	}

	gui_button->set_mouse_filter(Control::MOUSE_FILTER_PASS);

	gui_button->set_text(Utilities::std_to_godot_string(button.get_text()));

	if (button.get_font() != nullptr) {
		ret &= gui_button->set_gfx_font(button.get_font()) == OK;
	}

	if (try_create_shortcut_action_for_button(gui_button, shortcut_key_name) != OK) {
		WARN_PRINT(Utilities::format("Failed to create shortcut for GUI button '%s'", button_name));
	}

	gui_button->set_shortcut_feedback(false);
	gui_button->set_shortcut_in_tooltip(false);

	if (clicksound && !clicksound->get_file().empty()) {
		static auto gui_pressed = [](GUIButton* button, String const& file, float volume) {
			static auto on_audio_finished = [](AudioStreamPlayer* stream) {
				stream->queue_free();
			};
			Ref<AudioStreamWAV> audio_stream = SoundSingleton::get_singleton()->get_sound(file);

			if (audio_stream.is_valid()) {
				AudioStreamPlayer* asp = memnew(AudioStreamPlayer);
				asp->connect("finished", callable_mp_static(+on_audio_finished).bind(asp));
				if (AudioServer::get_singleton()->get_bus_index(OV_SNAME(SFX_BUS)) != -1) {
					asp->set_bus(OV_SNAME(SFX_BUS));
				}
				asp->set_stream(audio_stream);
				asp->set_volume_db(volume);
				asp->set_autoplay(true);
				button->get_tree()->get_root()->add_child(asp, false, Node::INTERNAL_MODE_BACK);
			}
		};

		gui_button->connect(
			"pressed",
			callable_mp_static(+gui_pressed)
				.bind(
					gui_button, Utilities::std_to_godot_string(clicksound->get_file().string()),
					static_cast<real_t>(clicksound->get_volume())
				)
		);
	}

	args.result = gui_button;
	return ret;
}

static bool generate_checkbox(generate_gui_args_t&& args) {
	GUI::Checkbox const& checkbox = static_cast<GUI::Checkbox const&>(args.element);

	const String checkbox_name = Utilities::std_to_godot_string(checkbox.get_name());
	const String shortcut_key_name = Utilities::std_to_godot_string(checkbox.get_shortcut());

	ERR_FAIL_NULL_V_MSG(checkbox.get_sprite(), false, Utilities::format("Null sprite for GUI checkbox %s", checkbox_name));

	GFX::IconTextureSprite const* texture_sprite = checkbox.get_sprite()->cast_to<GFX::IconTextureSprite>();

	ERR_FAIL_NULL_V_MSG(
		texture_sprite, false, Utilities::format(
			"Invalid sprite type %s for GUI checkbox %s", Utilities::std_to_godot_string(checkbox.get_sprite()->get_type()),
			checkbox_name
		)
	);

	GUIIconButton* gui_icon_button = nullptr;
	bool ret = new_control(gui_icon_button, checkbox, args.name);
	ERR_FAIL_NULL_V_MSG(
		gui_icon_button, false, Utilities::format("Failed to create GUIIconButton for GUI checkbox %s", checkbox_name)
	);

	gui_icon_button->set_toggle_mode(true);

	if (gui_icon_button->set_gfx_texture_sprite(texture_sprite) != OK) {
		UtilityFunctions::push_error("Error setting up GUIIconButton for GUI checkbox ", checkbox_name);
		ret = false;
	}

	gui_icon_button->set_text(Utilities::std_to_godot_string(checkbox.get_text()));

	if (checkbox.get_font() != nullptr) {
		ret &= gui_icon_button->set_gfx_font(checkbox.get_font()) == OK;
	}

	if (try_create_shortcut_action_for_button(gui_icon_button, shortcut_key_name) != OK) {
		WARN_PRINT(Utilities::format("Failed to create shortcut hotkey for GUI checkbox '%s'", checkbox_name));
	}

	gui_icon_button->set_shortcut_feedback(false);
	gui_icon_button->set_shortcut_in_tooltip(false);

	args.result = gui_icon_button;
	return ret;
}

static bool generate_text(generate_gui_args_t&& args) {
	GUI::Text const& text = static_cast<GUI::Text const&>(args.element);

	const String text_name = Utilities::std_to_godot_string(text.get_name());

	GUILabel* gui_label = nullptr;
	bool ret = new_control(gui_label, text, args.name);
	ERR_FAIL_NULL_V_MSG(gui_label, false, Utilities::format("Failed to create GUILabel for GUI text %s", text_name));

	gui_label->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	GameSingleton const* game_singleton = GameSingleton::get_singleton();
	GFX::Font::colour_codes_t const* override_colour_codes = game_singleton != nullptr
		? &game_singleton->get_definition_manager().get_ui_manager().get_universal_colour_codes() : nullptr;

	if (gui_label->set_gui_text(&text, override_colour_codes) != OK) {
		UtilityFunctions::push_error("Error initializing GUILabel for GUI text ", text_name);
		ret = false;
	}

	args.result = gui_label;
	return ret;
}

static bool generate_overlapping_elements(generate_gui_args_t&& args) {
	GUI::OverlappingElementsBox const& overlapping_elements = static_cast<GUI::OverlappingElementsBox const&>(args.element);

	const String overlapping_elements_name = Utilities::std_to_godot_string(overlapping_elements.get_name());

	GUIOverlappingElementsBox* box = nullptr;
	bool ret = new_control(box, overlapping_elements, args.name);
	ERR_FAIL_NULL_V_MSG(
		box, false,
		Utilities::format("Failed to create GUIOverlappingElementsBox for GUI overlapping elements %s", overlapping_elements_name)
	);
	box->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	if (box->set_gui_overlapping_elements_box(&overlapping_elements) != OK) {
		UtilityFunctions::push_error(
			"Error initializing GUIOverlappingElementsBox for GUI overlapping elements ", overlapping_elements_name
		);
		ret = false;
	}

	args.result = box;
	return ret;
}

static bool generate_listbox(generate_gui_args_t&& args) {
	GUI::ListBox const& listbox = static_cast<GUI::ListBox const&>(args.element);

	const String listbox_name = Utilities::std_to_godot_string(listbox.get_name());

	GUIListBox* gui_listbox = nullptr;
	bool ret = new_control(gui_listbox, listbox, args.name);
	ERR_FAIL_NULL_V_MSG(gui_listbox, false, Utilities::format("Failed to create GUIListBox for GUI listbox %s", listbox_name));

	if (gui_listbox->set_gui_listbox(&listbox) != OK) {
		UtilityFunctions::push_error("Error initializing GUIListBox for GUI listbox ", listbox_name);
		ret = false;
	}

	args.result = gui_listbox;
	return ret;
}

static bool generate_texteditbox(generate_gui_args_t&& args) {
	using namespace OpenVic::Utilities::literals;

	GUI::TextEditBox const& text_edit_box = static_cast<GUI::TextEditBox const&>(args.element);

	const String text_edit_box_name = Utilities::std_to_godot_string(text_edit_box.get_name());

	LineEdit* godot_line_edit = nullptr;
	bool ret = new_control(godot_line_edit, text_edit_box, args.name);
	ERR_FAIL_NULL_V_MSG(
		godot_line_edit, false, Utilities::format("Failed to create LineEdit for GUI text edit box %s", text_edit_box_name)
	);

	godot_line_edit->set_context_menu_enabled(false);
	godot_line_edit->set_caret_blink_enabled(true);
	godot_line_edit->set_focus_mode(Control::FOCUS_CLICK);

	godot_line_edit->set_text(Utilities::std_to_godot_string(text_edit_box.get_text()));

	static const Vector2 default_position_padding { -4.0_real, 1.0_real };
	static const Vector2 default_size_padding { 2.0_real, 2.0_real };
	const Vector2 border_size = Utilities::to_godot_fvec2(text_edit_box.get_border_size());
	const Vector2 max_size = Utilities::to_godot_fvec2(text_edit_box.get_size());
	godot_line_edit->set_position(godot_line_edit->get_position() + border_size + default_position_padding);
	godot_line_edit->set_custom_minimum_size(max_size - border_size - default_size_padding);

	static const Color caret_colour { 1.0_real, 0.0_real, 0.0_real };
	godot_line_edit->add_theme_color_override(OV_SNAME(caret_color), caret_colour);

	if (text_edit_box.get_font() != nullptr) {
		const StringName font_file = Utilities::std_to_godot_string(text_edit_box.get_font()->get_fontname());
		const Ref<Font> font = args.asset_manager.get_font(font_file);
		if (font.is_valid()) {
			godot_line_edit->add_theme_font_override(OV_SNAME(font), font);
		} else {
			UtilityFunctions::push_error("Failed to load font \"", font_file, "\" for GUI text edit box", text_edit_box_name);
			ret = false;
		}
		const Color colour = Utilities::to_godot_color(text_edit_box.get_font()->get_colour());
		godot_line_edit->add_theme_color_override(OV_SNAME(font_color), colour);
	}

	const StringName texture_file = Utilities::std_to_godot_string(text_edit_box.get_texture_file());
	if (!texture_file.is_empty()) {
		Ref<ImageTexture> texture = args.asset_manager.get_texture(texture_file);

		if (texture.is_valid()) {
			Ref<StyleBoxTexture> stylebox = AssetManager::make_stylebox_texture(texture, border_size);
			if (stylebox.is_valid()) {
				godot_line_edit->add_theme_stylebox_override(OV_SNAME(normal), stylebox);
			} else {
				UtilityFunctions::push_error("Failed to make StyleBoxTexture for GUI button ", text_edit_box_name);
				ret = false;
			}
		} else {
			UtilityFunctions::push_error(
				"Failed to load texture \"", texture_file, "\" for text edit box \"", text_edit_box_name, "\""
			);
			ret = false;
		}
	}

	Ref<StyleBoxEmpty> stylebox_empty;
	stylebox_empty.instantiate();
	if (stylebox_empty.is_valid()) {
		godot_line_edit->add_theme_stylebox_override(OV_SNAME(focus), stylebox_empty);
	} else {
		UtilityFunctions::push_error("Failed to create empty style box for focus of GUI text edit box ", text_edit_box_name);
		ret = false;
	}

	args.result = godot_line_edit;
	return ret;
}

static bool generate_scrollbar(generate_gui_args_t&& args) {
	GUI::Scrollbar const& scrollbar = static_cast<GUI::Scrollbar const&>(args.element);

	const String scrollbar_name = Utilities::std_to_godot_string(scrollbar.get_name());

	GUIScrollbar* gui_scrollbar = nullptr;
	bool ret = new_control(gui_scrollbar, scrollbar, args.name);
	ERR_FAIL_NULL_V_MSG(gui_scrollbar, false, Utilities::format("Failed to create GUIScrollbar for GUI scrollbar %s", scrollbar_name));

	if (gui_scrollbar->set_gui_scrollbar(&scrollbar) != OK) {
		UtilityFunctions::push_error("Error initializing GUIScrollbar for GUI scrollbar ", scrollbar_name);
		ret = false;
	}

	args.result = gui_scrollbar;
	return ret;
}

/* Forward declaration for use in generate_window. */
static bool generate_element(GUI::Element const* element, String const& name, AssetManager& asset_manager, Control*& result);

static bool generate_window(generate_gui_args_t&& args) {
	using namespace OpenVic::Utilities::literals;

	GUI::Window const& window = static_cast<GUI::Window const&>(args.element);

	// TODO - moveable, dontRender (disable visibility?)
	const String window_name = Utilities::std_to_godot_string(window.get_name());

	Panel* godot_panel = nullptr;
	bool ret = new_control(godot_panel, window, args.name);
	ERR_FAIL_NULL_V_MSG(godot_panel, false, Utilities::format("Failed to create Panel for GUI window %s", window_name));

	if (window.get_fullscreen()) {
		godot_panel->set_anchors_preset(godot::Control::PRESET_FULL_RECT);
	} else {
		godot_panel->set_custom_minimum_size(Utilities::to_godot_fvec2(window.get_size()));
	}

	Ref<StyleBoxEmpty> stylebox_empty;
	stylebox_empty.instantiate();
	if (stylebox_empty.is_valid()) {
		godot_panel->add_theme_stylebox_override(OV_SNAME(panel), stylebox_empty);
	} else {
		UtilityFunctions::push_error("Failed to create empty style box for background of GUI window ", window_name);
		ret = false;
	}
	godot_panel->set_mouse_filter(Control::MOUSE_FILTER_IGNORE);

	for (memory::unique_base_ptr<GUI::Element> const& element : window.get_window_elements()) {
		Control* node = nullptr;
		const bool element_ret = generate_element(element.get(), "", args.asset_manager, node);
		if (node != nullptr) {
			godot_panel->add_child(node);
		}
		if (!element_ret) {
			UtilityFunctions::push_error(
				"Errors generating GUI element ", Utilities::std_to_godot_string(element->get_name())
			);
			ret = false;
		}
	}

	args.result = godot_panel;
	return ret;
}

static bool generate_element(GUI::Element const* element, String const& name, AssetManager& asset_manager, Control*& result) {
	ERR_FAIL_NULL_V(element, false);
	static const ordered_map<std::string_view, bool (*)(generate_gui_args_t&&)> type_map {
		{ GUI::Icon::get_type_static(), &generate_icon },
		{ GUI::Button::get_type_static(), &generate_button },
		{ GUI::Checkbox::get_type_static(), &generate_checkbox },
		{ GUI::Text::get_type_static(), &generate_text },
		{ GUI::OverlappingElementsBox::get_type_static(), &generate_overlapping_elements },
		{ GUI::ListBox::get_type_static(), &generate_listbox },
		{ GUI::TextEditBox::get_type_static(), &generate_texteditbox },
		{ GUI::Scrollbar::get_type_static(), &generate_scrollbar },
		{ GUI::Window::get_type_static(), &generate_window }
	};
	const decltype(type_map)::const_iterator it = type_map.find(element->get_type());
	ERR_FAIL_COND_V_MSG(
		it == type_map.end(), false,
		Utilities::format("Invalid GUI element type: %s", Utilities::std_to_godot_string(element->get_type()))
	);
	return it->second({ *element, name, asset_manager, result });
}

bool UITools::generate_gui_element(
	GUI::Element const* element, String const& name, Control*& result
) {
	result = nullptr;
	AssetManager* asset_manager = AssetManager::get_singleton();
	ERR_FAIL_NULL_V(asset_manager, false);
	return generate_element(element, name, *asset_manager, result);
}

bool UITools::generate_gui_element(
	String const& gui_scene, String const& gui_element, String const& name, Control*& result
) {
	return generate_gui_element(get_gui_element(gui_scene, gui_element), name, result);
}
