#pragma once

#include <godot_cpp/classes/object.hpp>
#include <vector>
#include "Good.hpp"

namespace OpenVic2 {
	class LoadGoods {
		private:
			static bool extract_property_from_json(const godot::Variant& variant, std::vector<Good>& goods, int32_t index);

		public:
			static std::vector<Good> load_goods_from_disk(godot::String const& file_path);
	};
}