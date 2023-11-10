#pragma once

#include <algorithm>
#include <cassert>
#include <cstddef>
#include <iterator>
#include <string_view>
#include <type_traits>

namespace OpenVic {
	template<std::size_t N>
	struct StringLiteral {
		constexpr StringLiteral(const char (&str)[N]) {
			std::copy_n(str, N, value);
		}

		consteval StringLiteral(std::string_view string) {
			assert(string.size() == N);
			std::copy_n(string.begin(), N, value);
		}

		char value[N];
		static const constexpr std::integral_constant<std::size_t, N - 1> size {};

		struct iterator {
			using iterator_concept [[maybe_unused]] = std::contiguous_iterator_tag;
			using difference_type = std::ptrdiff_t;
			using element_type = const char; // element_type is a reserved name that must be used in the definition
			using pointer = element_type*;
			using reference = element_type&;

			constexpr iterator() = default;
			constexpr iterator(pointer p) : _ptr(p) {}
			constexpr reference operator*() const {
				return *_ptr;
			}
			constexpr pointer operator->() const {
				return _ptr;
			}
			constexpr auto& operator++() {
				_ptr++;
				return *this;
			}
			constexpr auto operator++(int) {
				auto tmp = *this;
				++(*this);
				return tmp;
			}
			constexpr iterator& operator+=(int i) {
				_ptr += i;
				return *this;
			}
			constexpr iterator operator+(const difference_type other) const {
				return _ptr + other;
			}
			constexpr friend iterator operator+(const difference_type value, const iterator& other) {
				return other + value;
			}
			constexpr iterator& operator--() {
				_ptr--;
				return *this;
			}
			constexpr iterator operator--(int) {
				iterator tmp = *this;
				--(*this);
				return tmp;
			}
			constexpr iterator& operator-=(int i) {
				_ptr -= i;
				return *this;
			}
			constexpr difference_type operator-(const iterator& other) const {
				return _ptr - other._ptr;
			}
			constexpr iterator operator-(const difference_type other) const {
				return _ptr - other;
			}
			friend iterator operator-(const difference_type value, const iterator& other) {
				return other - value;
			}
			constexpr reference operator[](difference_type idx) const {
				return _ptr[idx];
			}
			constexpr auto operator<=>(const iterator&) const = default; // three-way comparison C++20

		private:
			pointer _ptr;
		};

		constexpr iterator begin() const {
			return iterator { &value };
		}

		constexpr iterator end() const {
			return iterator { &value + N };
		}

		constexpr operator std::string_view() const {
			return std::string_view { value, N };
		}

		constexpr decltype(auto) data() const {
			return static_cast<std::string_view>(*this).data();
		}
	};
}
