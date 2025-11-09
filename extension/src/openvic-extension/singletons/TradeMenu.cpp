#include "MenuSingleton.hpp"

#include <godot_cpp/variant/utility_functions.hpp>

#include "openvic-extension/classes/GUILabel.hpp"
#include "openvic-extension/classes/GUIScrollbar.hpp"
#include "openvic-extension/core/StaticString.hpp"
#include "openvic-extension/singletons/GameSingleton.hpp"
#include "openvic-extension/singletons/PlayerSingleton.hpp"
#include "openvic-extension/utility/Utilities.hpp"

using namespace OpenVic;
using namespace godot;

/* TRADE MENU */

Dictionary MenuSingleton::get_trade_menu_good_categories_info() const {
	static const StringName good_index_key = "good_index";
	static const StringName current_price_key = "current_price";
	static const StringName price_change_key = "price_change";
	static const StringName demand_tooltip_key = "demand_tooltip";
	static const StringName trade_settings_key = "trade_settings";

	InstanceManager const* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	GoodInstanceManager const& good_instance_manager = instance_manager->get_good_instance_manager();
	GoodDefinitionManager const& good_definition_manager = good_instance_manager.get_good_definition_manager();

	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();

	Dictionary ret;

	for (GoodCategory const& good_category : good_definition_manager.get_good_categories()) {
		TypedArray<Dictionary> array;

		for (GoodDefinition const* good_definition : good_category.get_good_definitions()) {
			GoodInstance const& good_instance = good_instance_manager.get_good_instance_by_definition(*good_definition);

			if (!good_instance.is_trading_good()) {
				continue;
			}

			Dictionary good_dict;

			good_dict[good_index_key] = static_cast<int32_t>(good_definition->get_index());
			good_dict[current_price_key] = static_cast<real_t>(good_instance.get_price());
			good_dict[price_change_key] = static_cast<real_t>(good_instance.get_price_change_yesterday());

			{
				static const StringName in_demand_localisation_key = "TRADE_IN_DEMAND";
				static const StringName not_in_demand_localisation_key = "TRADE_NOT_IN_DEMAND";
				static const StringName supply_localisation_key = "SUPPLY";
				static const StringName demand_localisation_key = "DEMAND";
				static const StringName actual_bought_localisation_key = "ACTUAL_BOUGHT";

				const fixed_point_t supply = good_instance.get_total_supply_yesterday();
				const fixed_point_t demand = good_instance.get_total_demand_yesterday();

				good_dict[demand_tooltip_key] = tr(
					demand > supply ? in_demand_localisation_key : not_in_demand_localisation_key
				) + get_tooltip_separator() + tr(supply_localisation_key).replace(
					Utilities::get_short_value_placeholder(), Utilities::fixed_point_to_string_dp(supply, 3)
				) + "\n" + tr(demand_localisation_key).replace(
					Utilities::get_short_value_placeholder(), Utilities::fixed_point_to_string_dp(demand, 3)
				) + "\n" + tr(actual_bought_localisation_key).replace(
					Utilities::get_short_value_placeholder(), Utilities::fixed_point_to_string_dp(good_instance.get_quantity_traded_yesterday(), 3)
				);
			}

			if (country != nullptr) {
				CountryInstance::good_data_t const& good_data = country->get_good_data(good_instance);

				// Trade settings:
				//  - 1 bit: automated (1) or not (0)
				//  - 2 bit: buying (1) or not (0)
				//  - 4 bit: selling (1) or not (0)
				// The automated bit can be 0 or 1, regardless of the buying and selling bits' values.
				// The buying and selling bits can both be 0 or 1 and 0, but never both 1.

				int32_t trade_settings = TRADE_SETTING_NONE;

				if (good_data.is_selling) {
					if (good_data.stockpile_amount > good_data.stockpile_cutoff) {
						trade_settings = TRADE_SETTING_SELLING;
					}
				} else {
					if (good_data.stockpile_amount < good_data.stockpile_cutoff) {
						trade_settings = TRADE_SETTING_BUYING;
					}
				}

				if (good_data.is_automated) {
					trade_settings |= TRADE_SETTING_AUTOMATED;
				}

				good_dict[trade_settings_key] = trade_settings;
			}

			array.push_back(good_dict);
		}

		ret[Utilities::std_to_godot_string(good_category.get_identifier())] = std::move(array);
	}

	return ret;
}

Dictionary MenuSingleton::get_trade_menu_trade_details_info(
	int32_t trade_detail_good_index, GUIScrollbar* stockpile_cutoff_slider
) const {
	static const StringName trade_detail_good_name_key = "trade_detail_good_name";
	static const StringName trade_detail_good_price_key = "trade_detail_good_price";
	static const StringName trade_detail_good_base_price_key = "trade_detail_good_base_price";
	static const StringName trade_detail_price_history_key = "trade_detail_price_history";
	static const StringName trade_detail_is_automated_key = "trade_detail_is_automated";
	static const StringName trade_detail_is_selling_key = "trade_detail_is_selling"; // or buying (false)
	static const StringName trade_detail_slider_amount_key = "trade_detail_slider_amount"; // exponential good amount
	static const StringName trade_detail_government_needs_key = "trade_detail_government_needs";
	static const StringName trade_detail_army_needs_key = "trade_detail_army_needs";
	static const StringName trade_detail_overseas_needs_key = "trade_detail_overseas_needs";
	static const StringName trade_detail_factory_needs_key = "trade_detail_factory_needs";
	static const StringName trade_detail_pop_needs_key = "trade_detail_pop_needs";
	static const StringName trade_detail_available_key = "trade_detail_available";

	InstanceManager const* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});

	GoodInstance const* good_instance =
		instance_manager->get_good_instance_manager().get_good_instance_by_index(trade_detail_good_index);
	ERR_FAIL_NULL_V(good_instance, {});

	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();

	Dictionary ret;

	ret[trade_detail_good_name_key] = Utilities::std_to_godot_string(good_instance->get_identifier());
	ret[trade_detail_good_price_key] = static_cast<real_t>(good_instance->get_price());
	ret[trade_detail_good_base_price_key] = static_cast<real_t>(good_instance->get_good_definition().get_base_price());
	{
		ValueHistory<fixed_point_t> const& good_price_history = good_instance->get_price_history();

		PackedFloat32Array price_history;

		if (price_history.resize(good_price_history.size()) == OK) {
			for (size_t idx = 0; idx < good_price_history.size(); ++idx) {
				price_history[idx] = static_cast<real_t>(good_price_history[idx]);
			}

			ret[trade_detail_price_history_key] = std::move(price_history);
		} else {
			UtilityFunctions::push_error(
				"Failed to resize price history array to the correct size (",
				static_cast<int64_t>(good_price_history.size()), ")"
			);
		}
	}

	if (unlikely(country == nullptr)) {
		return ret;
	}

	CountryInstance::good_data_t const& good_data = country->get_good_data(*good_instance);

	ret[trade_detail_is_automated_key] = good_data.is_automated;
	ret[trade_detail_is_selling_key] = good_data.is_selling;
	if (stockpile_cutoff_slider != nullptr) {
		const int32_t index = calculate_slider_value_from_trade_menu_stockpile_cutoff(
			good_data.stockpile_cutoff,
			stockpile_cutoff_slider->get_max_value_scaled().truncate<int32_t>()
		);
		stockpile_cutoff_slider->set_scaled_value(index);
	}
	ret[trade_detail_slider_amount_key] = static_cast<real_t>(good_data.stockpile_cutoff);
	ret[trade_detail_government_needs_key] = static_cast<real_t>(good_data.government_needs);
	ret[trade_detail_army_needs_key] = static_cast<real_t>(good_data.army_needs);
	ret[OV_INAME("trade_detail_navy_needs")] = static_cast<real_t>(good_data.navy_needs);
	ret[trade_detail_overseas_needs_key] = static_cast<real_t>(good_data.overseas_maintenance);
	ret[trade_detail_factory_needs_key] = static_cast<real_t>(good_data.factory_demand);
	ret[trade_detail_pop_needs_key] = static_cast<real_t>(good_data.pop_demand);
	ret[trade_detail_available_key] = static_cast<real_t>(good_data.available_amount);

	return ret;
}

Dictionary MenuSingleton::get_trade_menu_tables_info() const {
	static const StringName good_producers_tooltips_key = "good_producers_tooltips";
	static const StringName good_trading_yesterday_tooltips_key = "good_trading_yesterday_tooltips";
	static const StringName government_needs_key = "government_needs";
	static const StringName factory_needs_key = "factory_needs";
	static const StringName pop_needs_key = "pop_needs";
	static const StringName market_activity_key = "market_activity";
	static const StringName stockpile_key = "stockpile";
	static const StringName common_market_key = "common_market";

	InstanceManager const* instance_manager = GameSingleton::get_singleton()->get_instance_manager();
	ERR_FAIL_NULL_V(instance_manager, {});
	GoodInstanceManager const& good_instance_manager = instance_manager->get_good_instance_manager();

	CountryInstance const* country = PlayerSingleton::get_singleton()->get_player_country();

	Dictionary ret;

	// This needs an entry for every good, even untradeable and unavailable ones, so we can look entries up with good indices
	PackedStringArray good_producers_tooltips;
	// TODO - replace test code with actual top producers
	CountryInstanceManager const& country_instance_manager = instance_manager->get_country_instance_manager();
	for (GoodInstance const& good : good_instance_manager.get_good_instances()) {
		static const StringName top_producers_localisation_key = "TRADE_TOP_PRODUCERS";

		String tooltip = tr(Utilities::std_to_godot_string(good.get_identifier())) + get_tooltip_separator() +
			tr(top_producers_localisation_key);

		for (size_t index = 0; index < 5; ++index) {
			CountryInstance const* country = country_instance_manager.get_country_instance_by_index(index + 1);

			ERR_CONTINUE(country == nullptr);

			static const String top_producer_template_string = "\n" + GUILabel::get_flag_marker() + "%s %s: %s";

			tooltip += Utilities::format(
				top_producer_template_string,
				Utilities::std_to_godot_string(country->get_identifier()),
				Utilities::get_country_name(*this, *country),
				Utilities::fixed_point_to_string_dp(fixed_point_t::parse(1000) / static_cast<int32_t>(index + 1), 2)
			);
		}

		good_producers_tooltips.push_back(tooltip);
	}
	ret[good_producers_tooltips_key] = std::move(good_producers_tooltips);

	if (unlikely(country == nullptr)) {
		return ret;
	}

	PackedStringArray good_trading_yesterday_tooltips;
	PackedVector2Array government_needs;
	PackedVector2Array factory_needs;
	PackedVector2Array pop_needs;
	PackedVector3Array market_activity;
	PackedVector3Array stockpile;
	PackedVector4Array common_market;

	for (auto const& [good, good_data] : country->get_goods_data()) {
		if (!good.is_trading_good()) {
			continue;
		}

		static const StringName stockpile_bought_localisation_key = "TRADE_STOCKPILE_BUY";
		static const StringName stockpile_sold_localisation_key = "TRADE_STOCKPILE_SOLD";
		static const String money_replace_key = "$MONEY$";
		static const String units_replace_key = "$UNITS$";

		fixed_point_t stockpile_change_yesterday = good_data.stockpile_change_yesterday;
		String tooltip;

		if (stockpile_change_yesterday > 0) {
			tooltip = tr(stockpile_bought_localisation_key);
		} else {
			tooltip = tr(stockpile_sold_localisation_key);
			stockpile_change_yesterday = -stockpile_change_yesterday;
		}

		good_trading_yesterday_tooltips.push_back(
			tooltip.replace(
				money_replace_key, Utilities::fixed_point_to_string_dp(stockpile_change_yesterday, 2)
			).replace(
				units_replace_key, Utilities::fixed_point_to_string_dp(stockpile_change_yesterday * good.get_price(), 2)
			)
		);

		const float good_index = good.get_good_definition().get_index();

		if (good_data.government_needs != fixed_point_t::_0) {
			government_needs.push_back({
				good_index,
				static_cast<real_t>(good_data.government_needs)
			});
		}

		if (good_data.factory_demand != fixed_point_t::_0) {
			factory_needs.push_back({
				good_index,
				static_cast<real_t>(good_data.factory_demand)
			});
		}

		if (good_data.pop_demand != fixed_point_t::_0) {
			pop_needs.push_back({
				good_index, static_cast<real_t>(good_data.pop_demand)
			});
		}

		market_activity.push_back({
			good_index,
			static_cast<real_t>(good_data.exported_amount.abs()),
			static_cast<real_t>(good_data.exported_amount * good.get_price())
		});

		stockpile.push_back({
			good_index,
			static_cast<real_t>(good_data.stockpile_amount),
			static_cast<real_t>(good_data.stockpile_change_yesterday)
		});

		// TODO - replace with actual common market data
		common_market.push_back({
			good_index,
			good_index * 100,
			-good_index,
			good_index * 10
		});
	}

	ret[good_trading_yesterday_tooltips_key] = std::move(good_trading_yesterday_tooltips);
	ret[government_needs_key] = std::move(government_needs);
	ret[factory_needs_key] = std::move(factory_needs);
	ret[pop_needs_key] = std::move(pop_needs);
	ret[market_activity_key] = std::move(market_activity);
	ret[stockpile_key] = std::move(stockpile);
	ret[common_market_key] = std::move(common_market);

	return ret;
}

float MenuSingleton::calculate_trade_menu_stockpile_cutoff_amount(GUIScrollbar const* slider) {
	ERR_FAIL_NULL_V(slider, 0.0f);

	return static_cast<float>(calculate_trade_menu_stockpile_cutoff_amount_fp(slider->get_value_scaled_fp()));
}
