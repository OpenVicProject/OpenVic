#pragma once

#include <string>
#include <godot_cpp/variant/string.hpp>

namespace OpenVic2 {
	class Good {
		public:
			using price_t = float;

			godot::String identifier;
			godot::String category;
			price_t cost;
			godot::String colour;
			bool isAvailableAtStart;
			bool isTradable;
			bool isMoney;
			bool hasOverseasPenalty;

			Good();
			Good(const godot::String& identifier, const godot::String& category, price_t cost, const godot::String& colour,
				bool isAvailable, bool isTradable, bool isMoney, bool hasOverseasPenalty);
			~Good();
	};
}