#pragma once
#include <optional>

template <typename T> using TEOption = std::optional<T>;
inline constexpr auto TENone = std::nullopt;
