#pragma once

#include <openvic-simulation/types/Signal.hpp>

namespace godot {
	struct String;
}

namespace OpenVic {
	struct CountryInstance;
	struct GUIIcon;
	struct GUIMaskedFlagButton;
	struct GUILabel;
	struct GUINode;
	struct fixed_point_t;

	struct ScoreOverview : public observer {
	private:
		GUIMaskedFlagButton& flag_button;
		GUIIcon& flag_icon;
		GUILabel& name_label;
		GUILabel& rank_label;
		GUILabel& prestige_label;
		GUILabel& prestige_rank_label;
		GUILabel& industrial_score_label;
		GUILabel& industrial_rank_label;
		GUILabel& military_score_label;
		GUILabel& military_rank_label;
		GUILabel& colonial_power_label;

		godot::String generate_prestige_tooltip(CountryInstance& country);
		void on_prestige_changed(const fixed_point_t new_prestige);

		godot::String generate_industrial_tooltip(CountryInstance& country);
		void on_industrial_score_changed(const fixed_point_t new_industrial_score);

		godot::String generate_military_tooltip(CountryInstance& country);
		void on_military_score_changed();
		void update_military_score(const fixed_point_t new_military_score);
	public:
		ScoreOverview(GUINode const& parent);
		void update();
	};
}