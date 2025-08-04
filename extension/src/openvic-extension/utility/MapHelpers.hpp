#pragma once

#include <tsl/ordered_map.h>

#include <openvic-simulation/types/IndexedFlatMap.hpp>

namespace OpenVic {
	// Helper aliases to deduce the key and value types from the MapType.
	// This allows the concept and function to work with both tsl::ordered_map and IndexedFlatMap.
	template <typename MapType>
	struct key_of {};

	template <typename KeyType, typename ValueType, typename... Args>
	struct key_of<tsl::ordered_map<KeyType, ValueType, Args...>> {
		using type = KeyType;
	};

	template <typename KeyType, typename ValueType>
	struct key_of<IndexedFlatMap<KeyType, ValueType>> {
		using type = KeyType;
	};

	template <typename MapType>
	using map_key_t = typename key_of<MapType>::type;

	template <typename MapType>
	struct value_of {};

	template <typename KeyType, typename ValueType, typename... Args>
	struct value_of<tsl::ordered_map<KeyType, ValueType, Args...>> {
		using type = ValueType;
	};

	template <typename KeyType, typename ValueType>
	struct value_of<IndexedFlatMap<KeyType, ValueType>> {
		using type = ValueType;
	};

	template <typename MapType>
	using map_value_t = typename value_of<MapType>::type;
}