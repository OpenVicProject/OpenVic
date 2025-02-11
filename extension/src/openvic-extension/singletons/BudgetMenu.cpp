#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUINode.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

Dictionary MenuSingleton::get_budget_menu_setup_info(GUIScrollbar* tariff_slider) const {
	static const StringName stratas_key = "stratas";
	static const StringName pop_types_by_strata_key = "pop_types_by_strata";
	static const StringName pop_sprites_by_type_key = "pop_sprites_by_type";
	static const StringName education_spending_pop_types_key = "education_spending_pop_types";
	static const StringName administration_spending_pop_types_key = "administration_spending_pop_types";
	// There are no social spending pop types even though an overlapping elements box for them exists in the GUI file
	// Non-administrative social_reforms reform group names
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";
	static const StringName military_spending_pop_types_key = "military_spending_pop_types";
	// Military spending pop type names
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// TODO - save the social and military spending subcategory ReformGroups and PopTypes for easier future lookup?

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	PopManager const& pop_manager = definition_manager.get_pop_manager();

	if (tariff_slider != nullptr) {
		tariff_slider->set_step_size_and_limits_fp(
			fixed_point_t::_1(),
			-fixed_point_t::_1() * SLIDER_SCALE,
			fixed_point_t::_1() * SLIDER_SCALE
		);
	}

	Dictionary dict;

	{
		PackedStringArray stratas;
		TypedArray<PackedByteArray> pop_types_by_strata;

		for (Strata const& strata : pop_manager.get_stratas()) {
			stratas.push_back(Utilities::std_to_godot_string(strata.get_identifier()));

			PackedByteArray pop_types;

			for (PopType const* pop_type : strata.get_pop_types()) {
				pop_types.push_back(pop_type->get_index());
			}

			pop_types_by_strata.push_back(pop_types);
		}

		dict[stratas_key] = std::move(stratas);
		dict[pop_types_by_strata_key] = std::move(pop_types_by_strata);
	}

	{
		using enum PopType::income_type_t;

		PackedByteArray pop_sprites_by_type;
		PackedByteArray education_spending_pop_types;
		PackedByteArray administration_spending_pop_types;
		PackedByteArray military_spending_pop_types;
		PackedStringArray military_spending_subcategories;

		for (PopType const& pop_type : pop_manager.get_pop_types()) {
			pop_sprites_by_type.push_back(pop_type.get_sprite());

			if (pop_type.has_income_type(EDUCATION)) {
				education_spending_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(ADMINISTRATION)) {
				administration_spending_pop_types.push_back(pop_type.get_index());
			}

			if (pop_type.has_income_type(MILITARY)) {
				military_spending_pop_types.push_back(pop_type.get_index());
				military_spending_subcategories.push_back(Utilities::std_to_godot_string(pop_type.get_identifier()));
			}
		}

		dict[pop_sprites_by_type_key] = std::move(pop_sprites_by_type);
		dict[education_spending_pop_types_key] = std::move(education_spending_pop_types);
		dict[administration_spending_pop_types_key] = std::move(administration_spending_pop_types);
		dict[military_spending_pop_types_key] = std::move(military_spending_pop_types);
		dict[military_spending_subcategories_key] = std::move(military_spending_subcategories);
	}

	{
		static constexpr std::string_view social_reforms_identifier = "social_reforms";

		ReformType const* social_reforms =
			definition_manager.get_politics_manager().get_issue_manager().get_reform_type_by_identifier(
				social_reforms_identifier
			);

		if (social_reforms != nullptr) {
			PackedStringArray social_spending_subcategories;

			for (ReformGroup const* reform_group : social_reforms->get_reform_groups()) {
				if (!reform_group->is_administrative()) {
					social_spending_subcategories.push_back(Utilities::std_to_godot_string(reform_group->get_identifier()));
				}
			}

			dict[social_spending_subcategories_key] = std::move(social_spending_subcategories);
		}
	}

	return dict;
}

enum struct needs_fulfilment_t : uint8_t {
	NO_NEEDS,
	LIFE_NEEDS_PARTIAL,
	LIFE_NEEDS,
	EVERYDAY_NEEDS,
	LUXURY_NEEDS,
	MAX_NEEDS_FULFILMENT_TYPE
};

static constexpr size_t NEEDS_FULFILMENT_COUNT = static_cast<size_t>(needs_fulfilment_t::MAX_NEEDS_FULFILMENT_TYPE);

template<typename T>
using needs_fulfilment_map_t = std::array<T, NEEDS_FULFILMENT_COUNT>;

static needs_fulfilment_map_t<Color> NEEDS_FULFILMENT_COLOURS {
	Color { 1.0f, 0.0f, 0.0f }, // NO_NEEDS - red
	Color { 1.0f, 1.0f, 0.0f }, // LIFE_NEEDS_PARTIAL - yellow
	Color { 0.0f, 1.0f, 0.0f }, // LIFE_NEEDS - green
	Color { 0.0f, 0.0f, 1.0f }, // EVERYDAY_NEEDS - blue
	Color { 0.0f, 1.0f, 1.0f }  // LUXURY_NEEDS - cyan
};

String MenuSingleton::_make_slider_range_limits_tooltip(
	CountryInstance const& country, ModifierEffect const& min_effect, ModifierEffect const& max_effect
) const {
	String slider_range_limits_tooltip = _make_modifier_effect_contributions_tooltip(country, max_effect);

	const String min_tooltip = _make_modifier_effect_contributions_tooltip(country, min_effect);

	if (!min_tooltip.is_empty()) {
		if (slider_range_limits_tooltip.is_empty()) {
			slider_range_limits_tooltip = min_tooltip;
		} else {
			slider_range_limits_tooltip += "\n" + min_tooltip;
		}
	}

	if (!slider_range_limits_tooltip.is_empty()) {
		slider_range_limits_tooltip = get_tooltip_separator().trim_suffix("\n") + slider_range_limits_tooltip;
	}

	return slider_range_limits_tooltip;
}

Dictionary MenuSingleton::get_budget_menu_info(
	TypedArray<GUIScrollbar> const& strata_tax_sliders,
	GUIScrollbar* land_spending_slider,
	GUIScrollbar* naval_spending_slider,
	GUIScrollbar* construction_spending_slider,
	GUIScrollbar* education_spending_slider,
	GUIScrollbar* administration_spending_slider,
	GUIScrollbar* social_spending_slider,
	GUIScrollbar* military_spending_slider,
	GUIScrollbar* tariff_slider
) const {
	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();
	if (country == nullptr) {
		return {};
	}

	DefinitionManager const& definition_manager = GameSingleton::get_singleton()->get_definition_manager();
	ModifierEffectCache const& modifier_effect_cache = definition_manager.get_modifier_manager().get_modifier_effect_cache();
	DefineManager const& define_manager = definition_manager.get_define_manager();

	static const String val_replace_key = "$VAL$";
	static const String value_replace_key = "$VALUE$";
	static const String need_replace_key = "$NEED$";
	static const String type_replace_key = "$TYPE$";

	static const needs_fulfilment_map_t<StringName> needs_fulfilment_localisation_keys {
		"NO_NEED",
		"SOME_LIFE_NEEDS",
		"LIFE_NEEDS",
		"EVERYDAY_NEEDS",
		"LUXURY_NEEDS"
	};

	needs_fulfilment_map_t<String> needs_fulfilment_localised_strings;
	for (size_t index = 0; index < NEEDS_FULFILMENT_COUNT; ++index) {
		needs_fulfilment_localised_strings[index] = tr(needs_fulfilment_localisation_keys[index]);
	}

	IndexedMap<PopType, pop_size_t> const& pop_type_distribution = country->get_pop_type_distribution();
	IndexedMap<PopType, needs_fulfilment_map_t<pop_size_t>> pop_type_needs_fulfilment_map { pop_type_distribution.get_keys() };

	std::vector<ProvinceInstance const*> gold_provinces;

	for (ProvinceInstance const* province : country->get_owned_provinces()) {
		GoodDefinition const* rgo_good = province->get_rgo_good();
		if (rgo_good != nullptr && rgo_good->get_is_money()) {
			gold_provinces.push_back(province);
		}

		for (Pop const& pop : province->get_pops()) {
			using enum needs_fulfilment_t;

			needs_fulfilment_map_t<pop_size_t>& pop_type_needs_fulfilment = pop_type_needs_fulfilment_map[*pop.get_type()];

			const pop_size_t pop_size = pop.get_size();

			const fixed_point_t life_needs_fulfilled = pop.get_life_needs_fulfilled();

			const fixed_point_t min_life_everyday = std::min(life_needs_fulfilled, pop.get_everyday_needs_fulfilled());

			const pop_size_t luxury_pop_size = pop_size * std::min(min_life_everyday, pop.get_luxury_needs_fulfilled());
			const pop_size_t luxury_and_everyday_pop_size = pop_size * min_life_everyday;

			pop_type_needs_fulfilment[static_cast<size_t>(LUXURY_NEEDS)] += luxury_pop_size;
			pop_type_needs_fulfilment[static_cast<size_t>(EVERYDAY_NEEDS)] += luxury_and_everyday_pop_size - luxury_pop_size;
			pop_type_needs_fulfilment[static_cast<size_t>(
				life_needs_fulfilled >= fixed_point_t::_1() ? LIFE_NEEDS :
					life_needs_fulfilled > fixed_point_t::_0() ? LIFE_NEEDS_PARTIAL : NO_NEEDS
			)] += pop_size - luxury_and_everyday_pop_size;
		}
	}

	Dictionary dict;

	{
		static const StringName pop_type_needs_tooltips_key = "pop_type_needs_tooltips";
		static const StringName pop_type_present_bools_key = "pop_type_present_bools";

		PackedStringArray pop_type_needs_tooltips;
		PackedByteArray pop_type_present_bools;

		if (
			pop_type_needs_tooltips.resize(pop_type_distribution.size()) == OK &&
			pop_type_present_bools.resize(pop_type_distribution.size()) == OK
		) {
			static const StringName no_pops_localisation_key = "NO_POPS_OF_TYPE";
			static const StringName getting_needs_localisation_key = "GETTING_NEEDS";
			static const StringName getting_only_needs_localisation_key = "GETTING_ONLY_NEEDS";

			const std::array<String, 2> getting_needs_strings {
				tr(getting_needs_localisation_key), tr(getting_only_needs_localisation_key)
			};

			static const needs_fulfilment_map_t<size_t> needs_fulfilment_getting_needs_string_index {
				0, 1, 1, 1, 0
			};

			for (int64_t index = 0; index < pop_type_distribution.size(); ++index) {
				PopType const& pop_type = pop_type_distribution(index);
				const pop_size_t pop_type_size = pop_type_distribution[index];

				String tooltip = tr(Utilities::std_to_godot_string(pop_type.get_identifier())) + get_tooltip_separator();
				const bool present = pop_type_size > 0;

				if (present) {
					needs_fulfilment_map_t<pop_size_t> const& pop_type_needs_fulfilment = pop_type_needs_fulfilment_map[index];

					for (size_t index = 0; index < NEEDS_FULFILMENT_COUNT; ++index) {
						if (index > 0) {
							tooltip += "\n";
						}
						tooltip += getting_needs_strings[needs_fulfilment_getting_needs_string_index[index]].replace(
							need_replace_key, needs_fulfilment_localised_strings[index]
						).replace(
							val_replace_key, Utilities::fixed_point_to_string_dp(
								fixed_point_t::_100() * pop_type_needs_fulfilment[index] / pop_type_size, 1
							)
						);
					}
				} else {
					tooltip += tr(no_pops_localisation_key);
				}

				pop_type_needs_tooltips[index] = std::move(tooltip);
				pop_type_present_bools[index] = present;
			}

			dict[pop_type_needs_tooltips_key] = std::move(pop_type_needs_tooltips);
			dict[pop_type_present_bools_key] = std::move(pop_type_present_bools);
		} else {
			UtilityFunctions::push_error("Failed to resize pop type needs tooltips and present bools arrays!");
		}
	}

	String total_income_tooltip;

	{
		// Tax
		static const StringName tax_efficiency_localisation_key = "BUDGET_TAX_EFFICIENCY";
		static const StringName tax_efficiency_from_tech_localisation_key = "BUDGET_TECH_TAX";
		static const StringName effective_tax_localisation_key = "BUDGET_TAX_EFFECT";

		const fixed_point_t tax_efficiency_from_tech =
			country->get_modifier_effect_value(*modifier_effect_cache.get_tax_eff()) / 100;

		const String tax_tooltip = tr(tax_efficiency_localisation_key).replace(
			value_replace_key, Utilities::fixed_point_to_string_dp((
				definition_manager.get_define_manager().get_country_defines().get_base_country_tax_efficiency() +
					country->get_modifier_effect_value(*modifier_effect_cache.get_tax_efficiency()) + tax_efficiency_from_tech
			) * 100, 2)
		) + "\n" + tr(tax_efficiency_from_tech_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(tax_efficiency_from_tech * 100, 2)
		) + "\n";

		const String effective_tax_localised_string = tr(effective_tax_localisation_key);

		IndexedMap<Strata, fixed_point_t> const& effective_tax_rate_by_strata = country->get_effective_tax_rate_by_strata();
		IndexedMap<Strata, String> strata_tax_tooltips { effective_tax_rate_by_strata.get_keys() };
		for (auto [strata, effective_tax_rate] : effective_tax_rate_by_strata) {
			strata_tax_tooltips[strata] = tax_tooltip + effective_tax_localised_string.replace(
				value_replace_key, Utilities::fixed_point_to_string_dp(effective_tax_rate * 100, 2)
			);
		}

		const String tax_range_tooltip = _make_slider_range_limits_tooltip(
			*country, *modifier_effect_cache.get_min_tax(), *modifier_effect_cache.get_max_tax()
		);

		IndexedMap<Strata, SliderValue> const& tax_rate_by_strata = country->get_tax_rate_slider_value_by_strata();
		const int64_t strata_tax_slider_count = std::min(
			strata_tax_sliders.size(), static_cast<int64_t>(tax_rate_by_strata.size())
		);
		for (int64_t index = 0; index < strata_tax_slider_count; ++index) {
			GUIScrollbar* strata_tax_slider = Object::cast_to<GUIScrollbar>(strata_tax_sliders[index]);

			if (strata_tax_slider != nullptr) {
				SliderValue const& strata_tax_slider_value = tax_rate_by_strata[index];

				strata_tax_slider->set_range_limits_and_value_from_slider_value(
					strata_tax_slider_value, SLIDER_SCALE, false
				);

				static const String tax_strata_template_string = "%s: " + GUILabel::get_colour_marker() + "Y%s%%" +
					GUILabel::get_colour_marker() + "!\n";

				strata_tax_slider->set_tooltip_string(
					vformat(
						tax_strata_template_string,
						tr("BUDGET_TAX_" + Utilities::std_to_godot_string(
							tax_rate_by_strata(index).get_identifier()).to_upper()
						),
						Utilities::fixed_point_to_string_dp(strata_tax_slider_value.get_value() * 100, 1)
					) + strata_tax_tooltips[index] + tax_range_tooltip
				);
			}
		}

		static const StringName tax_info_by_strata_key = "tax_info_by_strata";

		TypedArray<Dictionary> tax_info_by_strata;
		if (tax_info_by_strata.resize(effective_tax_rate_by_strata.size()) == OK) {
			static const StringName strata_needs_pie_chart_key = "strata_needs_pie_chart";
			static const StringName strata_tax_value_key = "strata_tax_value";
			static const StringName strata_tax_value_tooltip_key = "strata_tax_value_tooltip";

			static const StringName getting_no_needs_localisation_key = "BUDGET_STRATA_NO_NEED";
			static const StringName getting_needs_localisation_key = "BUDGET_STRATA_NEED";

			const String getting_needs_localised_string = tr(getting_needs_localisation_key);

			IndexedMap<Strata, pop_size_t> const& population_by_strata = country->get_population_by_strata();
			IndexedMap<PopType, fixed_point_t> const& taxable_income_by_pop_type = country->get_taxable_income_by_pop_type();

			for (int64_t index = 0; index < effective_tax_rate_by_strata.size(); ++index) {
				Strata const& strata = effective_tax_rate_by_strata(index);

				Dictionary strata_dict;

				GFXPieChartTexture::godot_pie_chart_data_t strata_needs_pie_chart_data;

				for (size_t index = 0; index < NEEDS_FULFILMENT_COUNT; ++index) {
					String slice_tooltip;
					pop_size_t total_pop_size = 0;

					for (PopType const* pop_type : strata.get_pop_types()) {
						const pop_size_t pop_type_size = pop_type_needs_fulfilment_map[*pop_type][index];

						if (pop_type_size <= 0) {
							continue;
						}

						total_pop_size += pop_type_size;

						if (!slice_tooltip.is_empty()) {
							slice_tooltip += "\n";
						}

						static const String slice_tooltip_template_string = GUILabel::get_colour_marker() + String { "Y%s " } +
							GUILabel::get_colour_marker() + "Y%s" + GUILabel::get_colour_marker() + "!: %s%%";
						static const String slice_pop_size_pre_suffix_string = GUILabel::get_colour_marker() + String { "!" };

						slice_tooltip += vformat(
							slice_tooltip_template_string,
							Utilities::int_to_string_suffixed(pop_type_size, slice_pop_size_pre_suffix_string),
							tr(Utilities::std_to_godot_string(pop_type->get_identifier())),
							Utilities::fixed_point_to_string_dp(
								fixed_point_t::_100() * pop_type_size / pop_type_distribution[*pop_type], 2
							)
						);
					}

					if (total_pop_size <= 0) {
						continue;
					}

					slice_tooltip = (index > 0 ? getting_needs_localised_string.replace(
						type_replace_key, needs_fulfilment_localised_strings[index]
					) : tr(getting_no_needs_localisation_key)).replace(
						val_replace_key, String::num_int64(100 * total_pop_size / population_by_strata[strata])
					) + get_tooltip_separator() + slice_tooltip;

					Dictionary slice_dict;

					slice_dict[GFXPieChartTexture::slice_identifier_key()] = needs_fulfilment_localisation_keys[index];
					slice_dict[GFXPieChartTexture::slice_tooltip_key()] = std::move(slice_tooltip);
					slice_dict[GFXPieChartTexture::slice_colour_key()] = NEEDS_FULFILMENT_COLOURS[index];
					slice_dict[GFXPieChartTexture::slice_weight_key()] = total_pop_size;

					strata_needs_pie_chart_data.push_back(slice_dict);
				}

				strata_dict[strata_needs_pie_chart_key] = std::move(strata_needs_pie_chart_data);

				const fixed_point_t strata_effective_tax_rate = effective_tax_rate_by_strata[strata];
				String strata_tax_value_tooltip;
				fixed_point_t strata_tax_value = fixed_point_t::_0();

				for (PopType const* pop_type : strata.get_pop_types()) {
					const fixed_point_t pop_type_taxable_income = taxable_income_by_pop_type[*pop_type];

					if (pop_type_taxable_income > fixed_point_t::_0()) {
						const fixed_point_t pop_type_tax_value = pop_type_taxable_income * strata_effective_tax_rate;

						strata_tax_value += pop_type_tax_value;

						static const String pop_type_tax_value_template_string = "%s " + GUILabel::get_colour_marker() +
							"Y%s" + GUILabel::get_currency_marker() + GUILabel::get_colour_marker() + "!\n";

						strata_tax_value_tooltip += vformat(
							pop_type_tax_value_template_string,
							tr(Utilities::std_to_godot_string(pop_type->get_identifier())),
							Utilities::fixed_point_to_string_dp(
								pop_type_tax_value, pop_type_tax_value < 10 ? 3 : pop_type_tax_value < 100 ? 2 : 1
							)
						);
					}
				}

				strata_dict[strata_tax_value_key] = strata_tax_value.to_float();

				total_income_tooltip += tr(
					"TAXES_" + Utilities::std_to_godot_string(strata.get_identifier()).to_upper()
				).replace(val_replace_key, Utilities::fixed_point_to_string_dp(strata_tax_value, 1)) + "\n";

				if (!strata_tax_value_tooltip.is_empty()) {
					strata_tax_value_tooltip += "\n";
				}
				strata_tax_value_tooltip += strata_tax_tooltips[index];
				strata_dict[strata_tax_value_tooltip_key] = std::move(strata_tax_value_tooltip);

				tax_info_by_strata[index] = std::move(strata_dict);
			}

			dict[tax_info_by_strata_key] = std::move(tax_info_by_strata);
		} else {
			UtilityFunctions::push_error("Failed to resize tax info by strata array!");
		}
	}

	{
		// Gold
		static const StringName gold_key = "gold";
		static const StringName gold_tooltip_key = "gold_tooltip";

		dict[gold_key] = country->get_gold_income().to_float();

		static const StringName gold_tooltip_localisation_key = "BUDGET_GOLD_INCOME_DESC";

		String gold_tooltip = tr(gold_tooltip_localisation_key);

		if (!gold_provinces.empty()) {
			gold_tooltip += get_tooltip_separator().trim_suffix("\n");

			std::sort(
				gold_provinces.begin(), gold_provinces.end(),
				[](ProvinceInstance const* lhs, ProvinceInstance const* rhs) -> bool {
					return lhs->get_rgo().get_output_quantity_yesterday() > rhs->get_rgo().get_output_quantity_yesterday();
				}
			);

			const fixed_point_t gold_to_cash_rate = define_manager.get_country_defines().get_gold_to_cash_rate();

			for (ProvinceInstance const* province : gold_provinces) {
				static const String gold_province_template_string = "\n%s: " + GUILabel::get_colour_marker() + "Y%s" +
					GUILabel::get_colour_marker() + "!" + GUILabel::get_currency_marker();

				gold_tooltip += vformat(
					gold_province_template_string,
					GUINode::format_province_name(Utilities::std_to_godot_string(province->get_identifier())),
					Utilities::fixed_point_to_string_dp(
						province->get_rgo().get_output_quantity_yesterday() * gold_to_cash_rate, 2
					)
				);
			}
		}

		dict[gold_tooltip_key] = std::move(gold_tooltip);
	}

	{
		// Total Income
		static const StringName total_income_key = "total_income";
		static const StringName total_income_tooltip_key = "total_income_tooltip";

		// TODO - replace these with actual values!
		const fixed_point_t total_income = 123;
		const fixed_point_t tariffs_income = 234;
		const fixed_point_t national_stockpile_sales_income = 345;

		dict[total_income_key] = total_income.to_float();

		static const StringName total_income_localisation_key = "BUDGET_TOTAL_INCOME";
		static const StringName tariffs_income_localisation_key = "TARIFFS_INCOME";
		static const StringName national_stockpile_income_localisation_key = "BUDGET_EXPORTS";
		static const StringName gold_income_localisation_key = "BUDGET_GOLD";

		total_income_tooltip = tr(total_income_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(total_income, 3)
		) + get_tooltip_separator() + total_income_tooltip + tr(tariffs_income_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(tariffs_income, 1)
		) + "\n" + tr(national_stockpile_income_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(national_stockpile_sales_income, 1)
		) + "\n" + tr(gold_income_localisation_key).replace(
			val_replace_key, Utilities::fixed_point_to_string_dp(country->get_gold_income(), 1)
		);

		dict[total_income_tooltip_key] = std::move(total_income_tooltip);
	}

	// Funds
	static const StringName national_bank_key = "national_bank";
	static const StringName national_bank_tooltip_key = "national_bank_tooltip";
	static const StringName total_funds_key = "total_funds";

	// TODO - dict[national_bank_key], dict[national_bank_tooltip_key], dict[total_funds_key]

	// Debt
	static const StringName total_debt_key = "total_debt";
	static const StringName interest_key = "interest";
	static const StringName repaying_debts_key = "repaying_debts";
	// TODO - should these be dictionaries from country name (or "SHADOWY_INVESTOR") to amount?
	// Also needed for pie charts. Table version has flag prefix, but not pie chart tooltips ("name" + get_tooltip_separator() + "amount£")
	static const StringName loans_taken_key = "loans_taken";
	static const StringName loans_given_key = "loans_given";

	// TODO - dict[total_debt_key], dict[interest_key], dict[repaying_debts_key]
	// TODO - dict[loans_taken_key], dict[loans_given_key] - ideally pie chart data that can be turned into table data?

	// Industrial Subsidies
	static const StringName industrial_subsidies_key = "industrial_subsidies";
	static const StringName industrial_subsidies_tooltip_key = "industrial_subsidies_tooltip";

	// TODO - dict[industrial_subsidies_key], dict[industrial_subsidies_tooltip_key]

	// National Stockpile
	static const StringName military_costs_key = "military_costs";
	static const StringName military_costs_tooltip_key = "military_costs_tooltip";

	// TODO - dict[military_costs_key], dict[military_costs_tooltip_key]

	static const StringName overseas_maintenance_key = "overseas_maintenance";
	static const StringName overseas_maintenance_tooltip_key = "overseas_maintenance_tooltip";

	// TODO - dict[overseas_maintenance_key], dict[overseas_maintenance_tooltip_key]

	static const StringName national_stockpile_today_key = "national_stockpile_today";
	static const StringName national_stockpile_today_tooltip_key = "national_stockpile_today_tooltip";
	// STOCKPILE_COST_ACTUAL;Today we �Yactually�! spent on stockpile:;

	// TODO - dict[national_stockpile_today_key], dict[national_stockpile_today_tooltip_key]

	{
		static const StringName land_spending_localisation_key = "MILITARY_SPENDINGS_LAND";
		static const StringName naval_spending_localisation_key = "MILITARY_SPENDINGS_NAVAL";
		static const StringName construction_spending_localisation_key = "MILITARY_SPENDINGS_PROJECTS";

		const String land_spending_localised_string = tr(land_spending_localisation_key);
		const String naval_spending_localised_string = tr(naval_spending_localisation_key);
		const String construction_spending_localised_string = tr(construction_spending_localisation_key);

		String land_spending_goods_tooltip;
		String naval_spending_goods_tooltip;
		String construction_spending_goods_tooltip;

		const auto write_good_tooltip_line = [this](
			String& tooltip, GoodDefinition const& good, fixed_point_t amount_bought, fixed_point_t amount_needed,
			fixed_point_t amount_spent
		) -> void {
			static const String all_bought_template_string = "\n" + GUILabel::get_colour_marker() + "Y%s" +
				GUILabel::get_colour_marker() + "!: " + GUILabel::get_colour_marker() + "G%s" + GUILabel::get_colour_marker() +
				"! (" + GUILabel::get_colour_marker() + "G%s" + GUILabel::get_currency_marker() +
				GUILabel::get_colour_marker() + "!)";
			static const String not_all_bought_template_string = "\n" + GUILabel::get_colour_marker() + "Y%s" +
				GUILabel::get_colour_marker() + "!: " + GUILabel::get_colour_marker() + "R%s" + GUILabel::get_colour_marker() +
				"!/" + GUILabel::get_colour_marker() + "R%s" + GUILabel::get_colour_marker() + "! (" +
				GUILabel::get_colour_marker() + "R%s" + GUILabel::get_currency_marker() + GUILabel::get_colour_marker() + "!)";

			if (amount_bought < amount_needed) {
				tooltip += vformat(
					not_all_bought_template_string,
					tr(Utilities::std_to_godot_string(good.get_identifier())),
					Utilities::fixed_point_to_string_dp(amount_bought, 4),
					Utilities::fixed_point_to_string_dp(amount_needed, 4),
					Utilities::fixed_point_to_string_dp(amount_spent, 4)
				);
			} else {
				tooltip += vformat(
					all_bought_template_string,
					tr(Utilities::std_to_godot_string(good.get_identifier())),
					Utilities::fixed_point_to_string_dp(amount_bought, 4),
					Utilities::fixed_point_to_string_dp(amount_spent, 4)
				);
			}
		};

		GoodDefinitionManager const& good_definition_manager =
			definition_manager.get_economy_manager().get_good_definition_manager();
		if (!good_definition_manager.good_definitions_empty()) {
			const size_t good_count = good_definition_manager.get_good_definition_count();

			write_good_tooltip_line(
				land_spending_goods_tooltip, *good_definition_manager.get_good_definition_by_index(4 % good_count), 1, 2, 3
			);
			write_good_tooltip_line(
				land_spending_goods_tooltip, *good_definition_manager.get_good_definition_by_index(6 % good_count), 3, 3, 1
			);

			write_good_tooltip_line(
				naval_spending_goods_tooltip, *good_definition_manager.get_good_definition_by_index(11 % good_count), 2, 3, 1
			);

			write_good_tooltip_line(
				construction_spending_goods_tooltip, *good_definition_manager.get_good_definition_by_index(11 % good_count),
				4, 4, 6
			);
		}

		static const StringName national_stockpile_tomorrow_key = "national_stockpile_tomorrow";
		static const StringName national_stockpile_tomorrow_tooltip_key = "national_stockpile_tomorrow_tooltip";

		// TODO - dict[national_stockpile_tomorrow_key] (RED IF CANNOT AFFORD!)

		static const StringName stockpile_tomorrow_localisation_key = "STOCKPILE_COST_ESTIMATED";

		dict[national_stockpile_tomorrow_tooltip_key] = tr(stockpile_tomorrow_localisation_key) + "\n\n" +
			land_spending_localised_string + land_spending_goods_tooltip + "\n\n" +
			naval_spending_localised_string + naval_spending_goods_tooltip + "\n\n" +
			construction_spending_localised_string + construction_spending_goods_tooltip;

		static const String stockpile_slider_tooltip_template_string = "%s " + GUILabel::get_colour_marker() + "Y%s%%%s";

		if (land_spending_slider != nullptr) {
			land_spending_slider->set_range_limits_and_value_from_slider_value(
				country->get_land_spending_slider_value(), SLIDER_SCALE, false
			);

			land_spending_slider->set_tooltip_string(vformat(
				stockpile_slider_tooltip_template_string,
				land_spending_localised_string,
				String::num_int64(country->get_land_spending_slider_value().get_value() * 100),
				land_spending_goods_tooltip
			));
		}
		if (naval_spending_slider != nullptr) {
			naval_spending_slider->set_range_limits_and_value_from_slider_value(
				country->get_naval_spending_slider_value(), SLIDER_SCALE, false
			);

			naval_spending_slider->set_tooltip_string(vformat(
				stockpile_slider_tooltip_template_string,
				naval_spending_localised_string,
				String::num_int64(country->get_naval_spending_slider_value().get_value() * 100),
				naval_spending_goods_tooltip
			));
		}
		if (construction_spending_slider != nullptr) {
			construction_spending_slider->set_range_limits_and_value_from_slider_value(
				country->get_construction_spending_slider_value(), SLIDER_SCALE, false
			);

			construction_spending_slider->set_tooltip_string(vformat(
				stockpile_slider_tooltip_template_string,
				construction_spending_localised_string,
				String::num_int64(country->get_construction_spending_slider_value().get_value() * 100),
				construction_spending_goods_tooltip
			));
		}
	}

	static const String spending_slider_template_string = "%s: " + GUILabel::get_colour_marker() + "Y%s%%";
	static const String range_limited_spending_slider_template_string = spending_slider_template_string +
		GUILabel::get_colour_marker() + "!%s";

	// Education
	if (education_spending_slider != nullptr) {
		education_spending_slider->set_value_from_slider_value(
			country->get_education_spending_slider_value(), SLIDER_SCALE, false
		);

		static const StringName education_spending_localisation_key = "DIST_EDUCATION";

		education_spending_slider->set_tooltip_string(
			vformat(
				spending_slider_template_string,
				tr(education_spending_localisation_key),
				Utilities::fixed_point_to_string_dp(country->get_education_spending_slider_value().get_value() * 100, 1)
			)
		);
	}
	static const StringName education_spending_value_key = "education_spending_value";

	// TODO - dict[education_spending_value_key] (RED IF CANNOT AFFORD, also maybe value tooltip for "We can only afford to pay...")

	// Administration
	static const StringName administrative_efficiency_key = "administrative_efficiency";
	static const StringName administrative_efficiency_tooltip_key = "administrative_efficiency_tooltip";

	// TODO - dict[administrative_efficiency_key], dict[administrative_efficiency_tooltip_key]

	if (administration_spending_slider != nullptr) {
		administration_spending_slider->set_value_from_slider_value(
			country->get_administration_spending_slider_value(), SLIDER_SCALE, false
		);

		static const StringName administration_spending_localisation_key = "DIST_ADMINISTRATION";

		administration_spending_slider->set_tooltip_string(
			vformat(
				spending_slider_template_string,
				tr(administration_spending_localisation_key),
				Utilities::fixed_point_to_string_dp(country->get_administration_spending_slider_value().get_value() * 100, 1)
			)
		);
	}
	static const StringName administration_spending_value_key = "administration_spending_value";

	// TODO - dict[administration_spending_value_key] (RED IF CANNOT AFFORD, also maybe value tooltip for "We can only afford to pay...")

	// Social Spending
	if (social_spending_slider != nullptr) {
		social_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_social_spending_slider_value(), SLIDER_SCALE, false
		);

		static const StringName social_spending_localisation_key = "DIST_SOCIAL";

		social_spending_slider->set_tooltip_string(
			vformat(
				range_limited_spending_slider_template_string,
				tr(social_spending_localisation_key),
				Utilities::fixed_point_to_string_dp(country->get_social_spending_slider_value().get_value() * 100, 1),
				_make_slider_range_limits_tooltip(
					*country, *modifier_effect_cache.get_min_social_spending(),
					*modifier_effect_cache.get_max_social_spending()
				)
			)
		);
	}
	static const StringName social_spending_value_key = "social_spending_value";
	static const StringName social_spending_subcategories_key = "social_spending_subcategories";

	// TODO - dict[social_spending_value_key] (RED IF CANNOT AFFORD, also maybe value tooltip for "We can only afford to pay...")
	// TODO - dict[social_spending_subcategories_key]

	// Miltiary Spending
	if (military_spending_slider != nullptr) {
		military_spending_slider->set_range_limits_and_value_from_slider_value(
			country->get_military_spending_slider_value(), SLIDER_SCALE, false
		);

		static const StringName military_spending_localisation_key = "DIST_DEFENCE";

		military_spending_slider->set_tooltip_string(
			vformat(
				range_limited_spending_slider_template_string,
				tr(military_spending_localisation_key),
				Utilities::fixed_point_to_string_dp(country->get_military_spending_slider_value().get_value() * 100, 1),
				_make_slider_range_limits_tooltip(
					*country, *modifier_effect_cache.get_min_military_spending(),
					*modifier_effect_cache.get_max_military_spending()
				)
			)
		);
	}
	static const StringName military_spending_value_key = "military_spending_value";
	static const StringName military_spending_subcategories_key = "military_spending_subcategories";

	// TODO - dict[military_spending_value_key] (RED IF CANNOT AFFORD, also maybe value tooltip for "We can only afford to pay...")
	// TODO - dict[military_spending_subcategories_key]

	// Total Expense
	static const StringName total_expense_key = "total_expense";
	static const StringName total_expense_tooltip_key = "total_expense_tooltip";

	// TODO - dict[total_expense_key], dict[total_expense_tooltip_key]
	// Tooltip includes: Education, Administration, Social Spending, Military Spending, Interest, National Stockpile Purchases

	// Tariffs
	if (tariff_slider != nullptr) {
		tariff_slider->set_range_limits_and_value_from_slider_value(
			country->get_tariff_rate_slider_value(), SLIDER_SCALE, false
		);
		// TODO - tooltip
	}
	static const StringName tariff_value_key = "tariff_value";

	// TODO - dict[tariff_value_key]

	// Diplomatic Balance
	static const StringName diplomatic_balance_key = "diplomatic_balance";
	static const StringName diplomatic_balance_tooltip_key = "diplomatic_balance_tooltip";

	// TODO - dict[diplomatic_balance_key], dict[diplomatic_balance_tooltip_key]

	// Projected Daily Balance
	static const StringName projected_daily_balance_key = "projected_daily_balance";

	// TODO - dict[projected_daily_balance_key]

	return dict;
}
