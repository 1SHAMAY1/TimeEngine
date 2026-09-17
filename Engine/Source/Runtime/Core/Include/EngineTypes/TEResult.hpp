#pragma once
#include <variant>
#include <utility>
#include <string>

class TEString;

template <typename E> struct TEUnexpected
{
    E error;
    explicit TEUnexpected(E err) : error(std::move(err)) {}
};

template <typename T, typename E = TEString> class TEResult
{
private:
    std::variant<T, E> m_Storage;
    bool m_HasValue;

public:
    TEResult(const T &val) : m_Storage(val), m_HasValue(true) {}
    TEResult(T &&val) : m_Storage(std::move(val)), m_HasValue(true) {}
    TEResult(const E &err) : m_Storage(err), m_HasValue(false) {}
    TEResult(E &&err) : m_Storage(std::move(err)), m_HasValue(false) {}
    TEResult(const TEUnexpected<E> &unexp) : m_Storage(unexp.error), m_HasValue(false) {}
    TEResult(TEUnexpected<E> &&unexp) : m_Storage(std::move(unexp.error)), m_HasValue(false) {}

    bool HasValue() const { return m_HasValue; }
    bool has_value() const { return m_HasValue; }
    explicit operator bool() const { return m_HasValue; }

    T &Value() { return std::get<0>(m_Storage); }
    const T &Value() const { return std::get<0>(m_Storage); }
    T &value() { return std::get<0>(m_Storage); }
    const T &value() const { return std::get<0>(m_Storage); }

    T &operator*() { return Value(); }
    const T &operator*() const { return Value(); }
    T *operator->() { return &Value(); }
    const T *operator->() const { return &Value(); }

    E &Error() { return std::get<1>(m_Storage); }
    const E &Error() const { return std::get<1>(m_Storage); }
    E &error() { return std::get<1>(m_Storage); }
    const E &error() const { return std::get<1>(m_Storage); }

    T ValueOr(T &&fallback) const { return m_HasValue ? std::get<0>(m_Storage) : std::forward<T>(fallback); }
    T value_or(T &&fallback) const { return m_HasValue ? std::get<0>(m_Storage) : std::forward<T>(fallback); }
    T value_or(const T &fallback) const { return m_HasValue ? std::get<0>(m_Storage) : fallback; }
};
