#pragma once

#include <cstdint>
#include <cstddef>
#include <type_traits>
#include <limits>
#include <cmath>
#include <optional>
#include <string>
#include <string_view>
#include <utility>
#include <memory>
#include <bit>
#include <concepts>
#include <sstream>
#include <iomanip>

// Forward declarations for Engine smart types and containers if available
class TEString;

// ============================================================================
// 1. Basic Result & Option Helper Types (Rust-inspired)
// ============================================================================
namespace TE {
    template <typename T>
    using TEOption = std::optional<T>;
    inline constexpr auto TENone = std::nullopt;

    template <typename T, typename E = std::string>
    class TEResult
    {
    private:
        bool m_HasValue;
        union {
            T m_Value;
            E m_Error;
        };

    public:
        constexpr TEResult(const T& val) : m_HasValue(true), m_Value(val) {}
        constexpr TEResult(T&& val) : m_HasValue(true), m_Value(std::move(val)) {}
        constexpr TEResult(bool /*isErr*/, const E& err) : m_HasValue(false), m_Error(err) {}
        constexpr TEResult(bool /*isErr*/, E&& err) : m_HasValue(false), m_Error(std::move(err)) {}

        ~TEResult() {
            if (m_HasValue) {
                m_Value.~T();
            } else {
                m_Error.~E();
            }
        }

        TEResult(const TEResult& other) : m_HasValue(other.m_HasValue) {
            if (m_HasValue) new (&m_Value) T(other.m_Value);
            else new (&m_Error) E(other.m_Error);
        }

        TEResult(TEResult&& other) noexcept : m_HasValue(other.m_HasValue) {
            if (m_HasValue) new (&m_Value) T(std::move(other.m_Value));
            else new (&m_Error) E(std::move(other.m_Error));
        }

        TEResult& operator=(const TEResult& other) {
            if (this != &other) {
                this->~TEResult();
                m_HasValue = other.m_HasValue;
                if (m_HasValue) new (&m_Value) T(other.m_Value);
                else new (&m_Error) E(other.m_Error);
            }
            return *this;
        }

        TEResult& operator=(TEResult&& other) noexcept {
            if (this != &other) {
                this->~TEResult();
                m_HasValue = other.m_HasValue;
                if (m_HasValue) new (&m_Value) T(std::move(other.m_Value));
                else new (&m_Error) E(std::move(other.m_Error));
            }
            return *this;
        }

        [[nodiscard]] constexpr bool IsOk() const noexcept { return m_HasValue; }
        [[nodiscard]] constexpr bool IsErr() const noexcept { return !m_HasValue; }
        [[nodiscard]] constexpr explicit operator bool() const noexcept { return m_HasValue; }

        [[nodiscard]] constexpr T& Value() & { return m_Value; }
        [[nodiscard]] constexpr const T& Value() const& { return m_Value; }
        [[nodiscard]] constexpr T&& Value() && { return std::move(m_Value); }

        [[nodiscard]] constexpr E& Error() & { return m_Error; }
        [[nodiscard]] constexpr const E& Error() const& { return m_Error; }
        [[nodiscard]] constexpr E&& Error() && { return std::move(m_Error); }

        [[nodiscard]] constexpr T ValueOr(T fallback) const {
            return m_HasValue ? m_Value : fallback;
        }

        static constexpr TEResult Ok(T val) { return TEResult(std::move(val)); }
        static constexpr TEResult Err(E err) { return TEResult(false, std::move(err)); }
    };
}

// ============================================================================
// 2. Type Traits & Detection
// ============================================================================
namespace TE::Detail {
    template <typename T>
    struct IsSmartRef : std::false_type {};
    template <typename T>
    struct IsSmartRef<std::shared_ptr<T>> : std::true_type {};

    template <typename T>
    struct IsSmartScope : std::false_type {};
    template <typename T, typename D>
    struct IsSmartScope<std::unique_ptr<T, D>> : std::true_type {};

    template <typename From, typename To>
    struct IsLosslessNumeric {
        static constexpr bool value = std::is_arithmetic_v<From> && std::is_arithmetic_v<To> &&
            (sizeof(To) >= sizeof(From)) &&
            (std::is_signed_v<To> == std::is_signed_v<From> || (std::is_signed_v<To> && !std::is_signed_v<From> && sizeof(To) > sizeof(From)));
    };
}

// ============================================================================
// 3. TEInt<T> - Smart Fixed & Native Integer Wrapper
// ============================================================================
template <typename T>
class TEInt
{
    static_assert(std::is_integral_v<T> && !std::is_same_v<T, bool>, "TEInt<T> requires an integral type (excluding bool).");

private:
    T m_Value = 0;

public:
    using ValueType = T;

    constexpr TEInt() noexcept = default;
    constexpr TEInt(T value) noexcept : m_Value(value) {}

    template <typename OtherInt>
        requires (std::is_integral_v<OtherInt> && !std::is_same_v<OtherInt, bool>)
    constexpr explicit(!std::is_same_v<T, OtherInt> && !Detail::IsLosslessNumeric<OtherInt, T>::value)
    TEInt(OtherInt value) noexcept : m_Value(static_cast<T>(value)) {}

    // Core Value Accessors
    [[nodiscard]] constexpr T Get() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T Value() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T& Raw() noexcept { return m_Value; }
    [[nodiscard]] constexpr const T& Raw() const noexcept { return m_Value; }

    [[nodiscard]] constexpr explicit operator T() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T operator*() const noexcept { return m_Value; }

    // Arithmetic Operators
    constexpr TEInt operator+(const TEInt& other) const noexcept { return TEInt(m_Value + other.m_Value); }
    constexpr TEInt operator-(const TEInt& other) const noexcept { return TEInt(m_Value - other.m_Value); }
    constexpr TEInt operator*(const TEInt& other) const noexcept { return TEInt(m_Value * other.m_Value); }
    constexpr TEInt operator/(const TEInt& other) const noexcept { return TEInt(m_Value / other.m_Value); }
    constexpr TEInt operator%(const TEInt& other) const noexcept { return TEInt(m_Value % other.m_Value); }

    constexpr TEInt& operator+=(const TEInt& other) noexcept { m_Value += other.m_Value; return *this; }
    constexpr TEInt& operator-=(const TEInt& other) noexcept { m_Value -= other.m_Value; return *this; }
    constexpr TEInt& operator*=(const TEInt& other) noexcept { m_Value *= other.m_Value; return *this; }
    constexpr TEInt& operator/=(const TEInt& other) noexcept { m_Value /= other.m_Value; return *this; }
    constexpr TEInt& operator%=(const TEInt& other) noexcept { m_Value %= other.m_Value; return *this; }

    // Bitwise Operators
    constexpr TEInt operator&(const TEInt& other) const noexcept { return TEInt(m_Value & other.m_Value); }
    constexpr TEInt operator|(const TEInt& other) const noexcept { return TEInt(m_Value | other.m_Value); }
    constexpr TEInt operator^(const TEInt& other) const noexcept { return TEInt(m_Value ^ other.m_Value); }
    constexpr TEInt operator~() const noexcept { return TEInt(~m_Value); }
    constexpr TEInt operator<<(size_t shift) const noexcept { return TEInt(m_Value << shift); }
    constexpr TEInt operator>>(size_t shift) const noexcept { return TEInt(m_Value >> shift); }

    // Comparisons
    constexpr auto operator<=>(const TEInt&) const = default;
    constexpr bool operator==(const TEInt&) const = default;

    // Checked Operations (Rust-style)
    [[nodiscard]] constexpr TE::TEOption<TEInt<T>> CheckedAdd(TEInt other) const noexcept {
        if constexpr (std::is_signed_v<T>) {
            if ((other.m_Value > 0 && m_Value > std::numeric_limits<T>::max() - other.m_Value) ||
                (other.m_Value < 0 && m_Value < std::numeric_limits<T>::min() - other.m_Value))
                return TE::TENone;
        } else {
            if (m_Value > std::numeric_limits<T>::max() - other.m_Value)
                return TE::TENone;
        }
        return TEInt(m_Value + other.m_Value);
    }

    [[nodiscard]] constexpr TE::TEOption<TEInt<T>> CheckedSub(TEInt other) const noexcept {
        if constexpr (std::is_signed_v<T>) {
            if ((other.m_Value < 0 && m_Value > std::numeric_limits<T>::max() + other.m_Value) ||
                (other.m_Value > 0 && m_Value < std::numeric_limits<T>::min() + other.m_Value))
                return TE::TENone;
        } else {
            if (m_Value < other.m_Value)
                return TE::TENone;
        }
        return TEInt(m_Value - other.m_Value);
    }

    [[nodiscard]] constexpr TE::TEOption<TEInt<T>> CheckedMul(TEInt other) const noexcept {
        if (m_Value == 0 || other.m_Value == 0) return TEInt(0);
        if constexpr (std::is_signed_v<T>) {
            if (m_Value > 0) {
                if (other.m_Value > 0 && m_Value > std::numeric_limits<T>::max() / other.m_Value) return TE::TENone;
                if (other.m_Value < 0 && other.m_Value < std::numeric_limits<T>::min() / m_Value) return TE::TENone;
            } else {
                if (other.m_Value > 0 && m_Value < std::numeric_limits<T>::min() / other.m_Value) return TE::TENone;
                if (other.m_Value < 0 && other.m_Value < std::numeric_limits<T>::max() / m_Value) return TE::TENone;
            }
        } else {
            if (m_Value > std::numeric_limits<T>::max() / other.m_Value) return TE::TENone;
        }
        return TEInt(m_Value * other.m_Value);
    }

    [[nodiscard]] constexpr TE::TEOption<TEInt<T>> CheckedDiv(TEInt other) const noexcept {
        if (other.m_Value == 0) return TE::TENone;
        if constexpr (std::is_signed_v<T>) {
            if (m_Value == std::numeric_limits<T>::min() && other.m_Value == -1) return TE::TENone;
        }
        return TEInt(m_Value / other.m_Value);
    }

    // Saturating Operations
    [[nodiscard]] constexpr TEInt SaturatingAdd(TEInt other) const noexcept {
        auto res = CheckedAdd(other);
        if (res.has_value()) return *res;
        return (other.m_Value > 0) ? TEInt(std::numeric_limits<T>::max()) : TEInt(std::numeric_limits<T>::min());
    }

    [[nodiscard]] constexpr TEInt SaturatingSub(TEInt other) const noexcept {
        auto res = CheckedSub(other);
        if (res.has_value()) return *res;
        return (other.m_Value > 0) ? TEInt(std::numeric_limits<T>::min()) : TEInt(std::numeric_limits<T>::max());
    }

    [[nodiscard]] constexpr TEInt SaturatingMul(TEInt other) const noexcept {
        auto res = CheckedMul(other);
        if (res.has_value()) return *res;
        bool sameSign = (m_Value > 0 && other.m_Value > 0) || (m_Value < 0 && other.m_Value < 0);
        return sameSign ? TEInt(std::numeric_limits<T>::max()) : TEInt(std::numeric_limits<T>::min());
    }

    // Wrapping Operations (Explicit modular arithmetic)
    [[nodiscard]] constexpr TEInt WrappingAdd(TEInt other) const noexcept {
        using UnsignedT = std::make_unsigned_t<T>;
        return TEInt(static_cast<T>(static_cast<UnsignedT>(m_Value) + static_cast<UnsignedT>(other.m_Value)));
    }

    [[nodiscard]] constexpr TEInt WrappingSub(TEInt other) const noexcept {
        using UnsignedT = std::make_unsigned_t<T>;
        return TEInt(static_cast<T>(static_cast<UnsignedT>(m_Value) - static_cast<UnsignedT>(other.m_Value)));
    }

    // Bit manipulation helpers
    [[nodiscard]] constexpr size_t CountOnes() const noexcept {
        using UnsignedT = std::make_unsigned_t<T>;
        return static_cast<size_t>(std::popcount(static_cast<UnsignedT>(m_Value)));
    }

    [[nodiscard]] constexpr size_t LeadingZeros() const noexcept {
        using UnsignedT = std::make_unsigned_t<T>;
        return static_cast<size_t>(std::countl_zero(static_cast<UnsignedT>(m_Value)));
    }

    [[nodiscard]] constexpr size_t TrailingZeros() const noexcept {
        using UnsignedT = std::make_unsigned_t<T>;
        return static_cast<size_t>(std::countr_zero(static_cast<UnsignedT>(m_Value)));
    }

    // Clamping
    [[nodiscard]] constexpr TEInt Clamp(TEInt minVal, TEInt maxVal) const noexcept {
        return (m_Value < minVal.m_Value) ? minVal : ((m_Value > maxVal.m_Value) ? maxVal : *this);
    }
};

// Standard Integer Aliases
using TEInt8      = TEInt<int8_t>;
using TEUInt8     = TEInt<uint8_t>;
using TEByte      = TEInt<uint8_t>;
using TEInt16     = TEInt<int16_t>;
using TEUInt16    = TEInt<uint16_t>;
using TEInt32     = TEInt<int32_t>;
using TEUInt32    = TEInt<uint32_t>;
using TEInt64     = TEInt<int64_t>;
using TEUInt64    = TEInt<uint64_t>;
using TELong      = TEInt<long>;
using TEULong     = TEInt<unsigned long>;
using TELongLong  = TEInt<long long>;
using TEULongLong = TEInt<unsigned long long>;
using TEISize     = TEInt<ptrdiff_t>;
using TEUSize     = TEInt<size_t>;
using TEPtrDiff   = TEInt<ptrdiff_t>;
using TEIntPtr    = TEInt<intptr_t>;
using TEUIntPtr   = TEInt<uintptr_t>;

// ============================================================================
// 4. TEFloat<T> - Smart Floating Point Wrapper (float, double, long double)
// ============================================================================
template <typename T>
class TEFloat
{
    static_assert(std::is_floating_point_v<T>, "TEFloat<T> requires a floating-point type.");

private:
    T m_Value = static_cast<T>(0.0);

public:
    using ValueType = T;

    constexpr TEFloat() noexcept = default;
    constexpr TEFloat(T value) noexcept : m_Value(value) {}

    template <typename OtherFloat>
        requires (std::is_floating_point_v<OtherFloat>)
    constexpr explicit(!std::is_same_v<T, OtherFloat> && sizeof(OtherFloat) > sizeof(T))
    TEFloat(OtherFloat value) noexcept : m_Value(static_cast<T>(value)) {}

    // Core Value Accessors
    [[nodiscard]] constexpr T Get() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T Value() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T& Raw() noexcept { return m_Value; }
    [[nodiscard]] constexpr const T& Raw() const noexcept { return m_Value; }

    [[nodiscard]] constexpr explicit operator T() const noexcept { return m_Value; }
    [[nodiscard]] constexpr T operator*() const noexcept { return m_Value; }

    // Arithmetic
    constexpr TEFloat operator+(const TEFloat& other) const noexcept { return TEFloat(m_Value + other.m_Value); }
    constexpr TEFloat operator-(const TEFloat& other) const noexcept { return TEFloat(m_Value - other.m_Value); }
    constexpr TEFloat operator*(const TEFloat& other) const noexcept { return TEFloat(m_Value * other.m_Value); }
    constexpr TEFloat operator/(const TEFloat& other) const noexcept { return TEFloat(m_Value / other.m_Value); }

    constexpr TEFloat& operator+=(const TEFloat& other) noexcept { m_Value += other.m_Value; return *this; }
    constexpr TEFloat& operator-=(const TEFloat& other) noexcept { m_Value -= other.m_Value; return *this; }
    constexpr TEFloat& operator*=(const TEFloat& other) noexcept { m_Value *= other.m_Value; return *this; }
    constexpr TEFloat& operator/=(const TEFloat& other) noexcept { m_Value /= other.m_Value; return *this; }

    // Precision & State Checks
    [[nodiscard]] bool IsNaN() const noexcept { return std::isnan(m_Value); }
    [[nodiscard]] bool IsFinite() const noexcept { return std::isfinite(m_Value); }
    [[nodiscard]] bool IsInfinite() const noexcept { return std::isinf(m_Value); }

    [[nodiscard]] constexpr bool NearlyEquals(TEFloat other, T tolerance = static_cast<T>(1e-5)) const noexcept {
        T diff = (m_Value > other.m_Value) ? (m_Value - other.m_Value) : (other.m_Value - m_Value);
        return diff <= tolerance;
    }

    [[nodiscard]] TE::TEOption<TEFloat<T>> CheckedDiv(TEFloat other, T epsilon = static_cast<T>(1e-8)) const noexcept {
        T absOther = (other.m_Value < 0) ? -other.m_Value : other.m_Value;
        if (absOther <= epsilon) return TE::TENone;
        return TEFloat(m_Value / other.m_Value);
    }

    // Math Functions
    [[nodiscard]] TEFloat Floor() const noexcept { return TEFloat(std::floor(m_Value)); }
    [[nodiscard]] TEFloat Ceil() const noexcept { return TEFloat(std::ceil(m_Value)); }
    [[nodiscard]] TEFloat Round() const noexcept { return TEFloat(std::round(m_Value)); }
    [[nodiscard]] TEFloat Trunc() const noexcept { return TEFloat(std::trunc(m_Value)); }
    [[nodiscard]] TEFloat Abs() const noexcept { return TEFloat(std::abs(m_Value)); }

    [[nodiscard]] constexpr TEFloat Clamp(TEFloat minVal, TEFloat maxVal) const noexcept {
        return (m_Value < minVal.m_Value) ? minVal : ((m_Value > maxVal.m_Value) ? maxVal : *this);
    }

    [[nodiscard]] constexpr TEFloat Lerp(TEFloat target, T alpha) const noexcept {
        return TEFloat(m_Value + (target.m_Value - m_Value) * alpha);
    }
};

// Standard Float Aliases
using TEFloat32    = TEFloat<float>;
using TEFloat64    = TEFloat<double>;
using TEDouble     = TEFloat<double>;
using TELongDouble = TEFloat<long double>;
using TEFloat128   = TEFloat<long double>;

// ============================================================================
// 5. TEEnum<E> - Advanced Smart Enum & Bitflag Wrapper
// ============================================================================
template <typename E>
class TEEnum
{
    static_assert(std::is_enum_v<E>, "TEEnum<E> requires an enumeration type.");

public:
    using UnderlyingType = std::underlying_type_t<E>;

private:
    UnderlyingType m_Value = 0;

public:
    constexpr TEEnum() noexcept = default;
    constexpr TEEnum(E value) noexcept : m_Value(static_cast<UnderlyingType>(value)) {}
    constexpr explicit TEEnum(UnderlyingType rawValue) noexcept : m_Value(rawValue) {}

    [[nodiscard]] constexpr E Value() const noexcept { return static_cast<E>(m_Value); }
    [[nodiscard]] constexpr UnderlyingType Underlying() const noexcept { return m_Value; }
    [[nodiscard]] constexpr explicit operator E() const noexcept { return Value(); }
    [[nodiscard]] constexpr explicit operator UnderlyingType() const noexcept { return m_Value; }

    // Bitflag Queries
    [[nodiscard]] constexpr bool HasFlag(E flag) const noexcept {
        UnderlyingType f = static_cast<UnderlyingType>(flag);
        return (m_Value & f) == f;
    }

    [[nodiscard]] constexpr bool HasFlag(TEEnum flag) const noexcept {
        return (m_Value & flag.m_Value) == flag.m_Value;
    }

    [[nodiscard]] constexpr bool HasAnyFlags(TEEnum flags) const noexcept {
        return (m_Value & flags.m_Value) != 0;
    }

    [[nodiscard]] constexpr bool HasAllFlags(TEEnum flags) const noexcept {
        return (m_Value & flags.m_Value) == flags.m_Value;
    }

    [[nodiscard]] constexpr bool IsEmpty() const noexcept {
        return m_Value == 0;
    }

    [[nodiscard]] constexpr size_t CountSetBits() const noexcept {
        using UnsignedUnderlying = std::make_unsigned_t<UnderlyingType>;
        return static_cast<size_t>(std::popcount(static_cast<UnsignedUnderlying>(m_Value)));
    }

    // Flag Mutations
    constexpr TEEnum& SetFlag(E flag) noexcept {
        m_Value |= static_cast<UnderlyingType>(flag);
        return *this;
    }

    constexpr TEEnum& ClearFlag(E flag) noexcept {
        m_Value &= ~static_cast<UnderlyingType>(flag);
        return *this;
    }

    constexpr TEEnum& ToggleFlag(E flag) noexcept {
        m_Value ^= static_cast<UnderlyingType>(flag);
        return *this;
    }

    // Fluent Modifiers
    [[nodiscard]] constexpr TEEnum WithFlag(E flag) const noexcept {
        return TEEnum(m_Value | static_cast<UnderlyingType>(flag));
    }

    [[nodiscard]] constexpr TEEnum WithoutFlag(E flag) const noexcept {
        return TEEnum(m_Value & ~static_cast<UnderlyingType>(flag));
    }

    // Bitwise Operators
    constexpr TEEnum operator|(TEEnum other) const noexcept { return TEEnum(m_Value | other.m_Value); }
    constexpr TEEnum operator&(TEEnum other) const noexcept { return TEEnum(m_Value & other.m_Value); }
    constexpr TEEnum operator^(TEEnum other) const noexcept { return TEEnum(m_Value ^ other.m_Value); }
    constexpr TEEnum operator~() const noexcept { return TEEnum(~m_Value); }

    constexpr TEEnum operator|(E other) const noexcept { return TEEnum(m_Value | static_cast<UnderlyingType>(other)); }
    constexpr TEEnum operator&(E other) const noexcept { return TEEnum(m_Value & static_cast<UnderlyingType>(other)); }
    constexpr TEEnum operator^(E other) const noexcept { return TEEnum(m_Value ^ static_cast<UnderlyingType>(other)); }

    constexpr TEEnum& operator|=(TEEnum other) noexcept { m_Value |= other.m_Value; return *this; }
    constexpr TEEnum& operator&=(TEEnum other) noexcept { m_Value &= other.m_Value; return *this; }
    constexpr TEEnum& operator^=(TEEnum other) noexcept { m_Value ^= other.m_Value; return *this; }

    constexpr auto operator<=>(const TEEnum&) const = default;
    constexpr bool operator==(const TEEnum&) const = default;
};

// ============================================================================
// 6. TEStruct<T> - Type-Safe Invariant Struct Wrapper
// ============================================================================
template <typename T>
class TEStruct
{
    static_assert(std::is_class_v<T>, "TEStruct<T> requires a class or struct type.");

private:
    T m_Data;

public:
    using StructType = T;

    constexpr TEStruct() = default;
    constexpr explicit TEStruct(const T& data) : m_Data(data) {}
    constexpr explicit TEStruct(T&& data) noexcept : m_Data(std::move(data)) {}

    template <typename... Args>
    constexpr explicit TEStruct(Args&&... args) : m_Data(std::forward<Args>(args)...) {}

    [[nodiscard]] constexpr const T& Read() const noexcept { return m_Data; }
    [[nodiscard]] constexpr const T* operator->() const noexcept { return &m_Data; }
    [[nodiscard]] constexpr const T& operator*() const noexcept { return m_Data; }

    // Direct access
    [[nodiscard]] constexpr const T& Get() const noexcept { return m_Data; }
    [[nodiscard]] constexpr T& Raw() noexcept { return m_Data; }

    // Mutator with validation hook
    template <typename Fn>
    constexpr void Mutate(Fn&& fn) {
        std::forward<Fn>(fn)(m_Data);
    }

    [[nodiscard]] constexpr TEStruct Clone() const {
        return TEStruct(m_Data);
    }
};

// ============================================================================
// 7. TEClass<T> - Smart Class / Metaclass & Instance Wrapper
// ============================================================================
template <typename T>
class TEClass
{
    static_assert(std::is_class_v<T>, "TEClass<T> requires a class or struct type.");

public:
    using ClassType = T;

    constexpr TEClass() noexcept = default;

    // Static Metaclass / Type Information
    [[nodiscard]] static constexpr const char* GetName() noexcept {
        if constexpr (requires { T::StaticClassName; }) {
            return T::StaticClassName.c_str();
        } else {
            return "TEClass";
        }
    }

    [[nodiscard]] static constexpr size_t GetSize() noexcept {
        return sizeof(T);
    }

    [[nodiscard]] static constexpr size_t GetAlign() noexcept {
        return alignof(T);
    }

    template <typename Base>
    [[nodiscard]] static constexpr bool IsChildOf() noexcept {
        return std::is_base_of_v<Base, T>;
    }

    // Smart Factory Instantiation
    template <typename... Args>
    [[nodiscard]] static std::shared_ptr<T> CreateRef(Args&&... args) {
        return std::make_shared<T>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]] static std::unique_ptr<T> CreateScope(Args&&... args) {
        return std::make_unique<T>(std::forward<Args>(args)...);
    }

    template <typename... Args>
    [[nodiscard]] static T Create(Args&&... args) {
        return T(std::forward<Args>(args)...);
    }
};

// ============================================================================
// 8. TECast - Universal Zero/Least Overhead Cast System
// ============================================================================
namespace Detail {
    template <typename To, typename From>
    struct CastDispatcher {
        static constexpr bool is_same = std::is_same_v<std::decay_t<To>, std::decay_t<From>>;
        static constexpr bool is_both_arithmetic = std::is_arithmetic_v<std::decay_t<To>> && std::is_arithmetic_v<std::decay_t<From>>;
        static constexpr bool is_smart_ref = Detail::IsSmartRef<std::decay_t<From>>::value;
        static constexpr bool is_smart_scope = Detail::IsSmartScope<std::decay_t<From>>::value;
        static constexpr bool is_ptr_both = std::is_pointer_v<std::decay_t<From>> && std::is_pointer_v<std::decay_t<To>>;
        static constexpr bool is_enum_to_int = std::is_enum_v<std::decay_t<From>> && std::is_integral_v<std::decay_t<To>>;
        static constexpr bool is_int_to_enum = std::is_integral_v<std::decay_t<From>> && std::is_enum_v<std::decay_t<To>>;
    };
}

// Universal TECast<To>(From) Function Template
template <typename To, typename From>
[[nodiscard]] constexpr decltype(auto) TECast(From&& from) noexcept
{
    using DecayedFrom = std::decay_t<From>;
    using DecayedTo = std::decay_t<To>;

    // 1. Identity Cast
    if constexpr (Detail::CastDispatcher<To, From>::is_same) {
        return std::forward<From>(from);
    }
    // 2. Arithmetic Primitives (Zero-overhead inline static_cast)
    else if constexpr (Detail::CastDispatcher<To, From>::is_both_arithmetic) {
        return static_cast<To>(from);
    }
    // 3. Enum Conversions (Zero-overhead inline static_cast)
    else if constexpr (Detail::CastDispatcher<To, From>::is_enum_to_int || Detail::CastDispatcher<To, From>::is_int_to_enum) {
        return static_cast<To>(from);
    }
    // 4. Smart Ref Downcast (Least-overhead dynamic/static pointer cast)
    else if constexpr (Detail::CastDispatcher<To, From>::is_smart_ref) {
        using TargetElement = typename DecayedTo::element_type;
        return std::dynamic_pointer_cast<TargetElement>(from);
    }
    // 5. Raw Pointer Polymorphic Downcast
    else if constexpr (Detail::CastDispatcher<To, From>::is_ptr_both) {
        using BaseType = std::remove_pointer_t<DecayedFrom>;
        if constexpr (std::is_polymorphic_v<BaseType>) {
            return dynamic_cast<To>(from);
        } else {
            return static_cast<To>(from);
        }
    }
    // 6. Generic Fallback
    else {
        return static_cast<To>(std::forward<From>(from));
    }
}

// Fallible Checked Cast (Boundary & Overflow Validation returning TEResult)
template <typename To>
struct TECastCheck
{
    template <typename From>
    [[nodiscard]] static constexpr TE::TEResult<To, std::string> Try(From from) noexcept
    {
        if constexpr (std::is_integral_v<From> && std::is_integral_v<To>) {
            if constexpr (std::is_signed_v<From> == std::is_signed_v<To>) {
                if (from < std::numeric_limits<To>::min() || from > std::numeric_limits<To>::max())
                    return TE::TEResult<To, std::string>::Err("Numeric cast out of range.");
            } else if constexpr (std::is_signed_v<From> && !std::is_signed_v<To>) {
                if (from < 0 || static_cast<std::make_unsigned_t<From>>(from) > std::numeric_limits<To>::max())
                    return TE::TEResult<To, std::string>::Err("Negative value cast to unsigned or out of range.");
            } else {
                if (from > static_cast<std::make_unsigned_t<To>>(std::numeric_limits<To>::max()))
                    return TE::TEResult<To, std::string>::Err("Unsigned value exceeds signed target maximum.");
            }
            return TE::TEResult<To, std::string>::Ok(static_cast<To>(from));
        } else if constexpr (std::is_floating_point_v<From> && std::is_integral_v<To>) {
            if (std::isnan(from) || std::isinf(from))
                return TE::TEResult<To, std::string>::Err("Cannot cast NaN or Infinity to integer.");
            if (from < static_cast<From>(std::numeric_limits<To>::min()) || from > static_cast<From>(std::numeric_limits<To>::max()))
                return TE::TEResult<To, std::string>::Err("Float value exceeds integer target bounds.");
            return TE::TEResult<To, std::string>::Ok(static_cast<To>(from));
        } else {
            return TE::TEResult<To, std::string>::Ok(static_cast<To>(from));
        }
    }
};
