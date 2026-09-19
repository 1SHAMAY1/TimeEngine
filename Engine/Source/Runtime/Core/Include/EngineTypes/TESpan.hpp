#pragma once
#include <span>
#include <string_view>

template <typename T> using TESpan = std::span<T>;
using TEStringView = std::string_view;
