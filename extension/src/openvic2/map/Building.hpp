#pragma once

#include <vector>

#include "openvic2/Types.hpp"
#include "openvic2/Date.hpp"

namespace OpenVic2 {
	struct BuildingManager;
	struct BuildingType;

	/* REQUIREMENTS:
	 * MAP-11, MAP-72, MAP-73
	 * MAP-12, MAP-75, MAP-76
	 * MAP-13, MAP-78, MAP-79
	 */
	struct Building : HasIdentifier {
		friend struct BuildingManager;

		using level_t = int8_t;

		enum class ExpansionState { CannotExpand, CanExpand, Preparing, Expanding };
	private:
		BuildingType const& type;
		level_t level = 0;
		ExpansionState expansion_state = ExpansionState::CannotExpand;
		Date start, end;
		float expansion_progress;

		Building(BuildingType const& new_type);

		bool _can_expand() const;
	public:
		BuildingType const& get_type() const;
		level_t get_level() const;
		ExpansionState get_expansion_state() const;
		Date const& get_start_date() const;
		Date const& get_end_date() const;
		float get_expansion_progress() const;

		return_t expand();
		void update_state(Date const& today);
		void tick(Date const& today);
	};

	struct BuildingType : HasIdentifier {
		friend struct BuildingManager;
	private:
		Building::level_t max_level;
		Timespan build_time;

		BuildingType(std::string const& new_identifier, Building::level_t new_max_level, Timespan new_build_time);
	public:
		Building::level_t get_max_level() const;
		Timespan get_build_time() const;
	};

	struct BuildingManager {
	private:
		static const char building_types_name[];
		IdentifierRegistry<BuildingType, building_types_name> building_types;
	public:
		return_t add_building_type(std::string const& identifier, Building::level_t max_level, Timespan build_time);
		void lock_building_types();
		BuildingType const* get_building_type_by_identifier(std::string const& identifier) const;
		void generate_province_buildings(std::vector<Building>& buildings) const;
	};
}
