#pragma once

#include <godot_cpp/variant/string.hpp>
#include <string>

namespace OpenVic2 {
	class Good {
		public:
			godot::String identifier;
			godot::String category;
			float_t cost;
			godot::String colour;
			bool isAvailableAtStart;
			bool isTradable;
			bool isMoney;
			bool hasOverseasPenalty;

			Good();
			Good(const godot::String& identifier, const godot::String& category, float_t cost, const godot::String& colour,
				 bool isAvailable, bool isTradable, bool isMoney, bool hasOverseasPenalty);
			~Good();
	};
}