#pragma once

#include "openvic2/GameAdvancementHook.hpp"
#include "openvic2/map/Map.hpp"

namespace OpenVic2 {
	struct GameManager {
		using state_updated_func_t = std::function<void()>;

		Map map;
		BuildingManager building_manager;
		GameAdvancementHook clock;
	private:
		Date today;
		state_updated_func_t state_updated;
		bool needs_update = true;

		void set_needs_update();
		void update_state();
		void tick();
	public:
		GameManager(state_updated_func_t state_updated_callback);

		void finished_loading_data();

		Date const& get_today() const;
		return_t expand_building(Province::index_t province_index, std::string const& building_type_identifier);
	};
}
