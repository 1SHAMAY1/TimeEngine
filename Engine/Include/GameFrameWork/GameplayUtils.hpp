#pragma once
#include "Core/PreRequisites.h"
#include "GameFrameWork/TFunctionLibrary.hpp"
#include "Utils/MathUtils.hpp"
#include <algorithm>
#include <functional>
#include <initializer_list>
#include <optional>
#include <span>
#include <stdexcept>
#include <string>
#include <string_view>
#include <type_traits>
#include <unordered_map>
#include <variant>
#include <vector>
#if __has_include(<expected>)
#include <expected>
#endif

class Scene;
class Entity;
class EntityManager;
class TComponent;
class TObject;

// ==========================================
// Modern Type Utilities & Safe Views
// ==========================================

template <typename T> using TEOption = std::optional<T>;
inline constexpr auto TENone = std::nullopt;

template <typename T> using TESpan = std::span<T>;

using TEStringView = std::string_view;

#if defined(__cpp_lib_expected) && __cpp_lib_expected >= 202202L
template <typename T, typename E = TEString> using TEResult = std::expected<T, E>;
template <typename E> using TEUnexpected = std::unexpected<E>;
#else
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
    TEResult(const TEUnexpected<E> &unexp) : m_Storage(unexp.error), m_HasValue(false) {}
    TEResult(TEUnexpected<E> &&unexp) : m_Storage(std::move(unexp.error)), m_HasValue(false) {}

    bool HasValue() const { return m_HasValue; }
    explicit operator bool() const { return m_HasValue; }

    T &Value() { return std::get<0>(m_Storage); }
    const T &Value() const { return std::get<0>(m_Storage); }

    T &operator*() { return Value(); }
    const T &operator*() const { return Value(); }
    T *operator->() { return &Value(); }
    const T *operator->() const { return &Value(); }

    E &Error() { return std::get<1>(m_Storage); }
    const E &Error() const { return std::get<1>(m_Storage); }

    T ValueOr(T &&fallback) const { return m_HasValue ? std::get<0>(m_Storage) : std::forward<T>(fallback); }
};
#endif

// ==========================================
// TEArray - Dynamic Engine Container Template
// ==========================================
template <typename T> class TEArray
{
private:
    std::vector<T> m_Elements;

public:
    using value_type = T;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using reference = typename std::vector<T>::reference;
    using const_reference = typename std::vector<T>::const_reference;
    using pointer = T *;
    using const_pointer = const T *;
    using iterator = typename std::vector<T>::iterator;
    using const_iterator = typename std::vector<T>::const_iterator;
    using reverse_iterator = typename std::vector<T>::reverse_iterator;
    using const_reverse_iterator = typename std::vector<T>::const_reverse_iterator;

    TEArray() = default;
    TEArray(std::initializer_list<T> initList) : m_Elements(initList) {}
    explicit TEArray(size_t count) : m_Elements(count) {}
    TEArray(size_t count, const T &value) : m_Elements(count, value) {}

    template <typename InputIt> TEArray(InputIt first, InputIt last) : m_Elements(first, last) {}

    // Implicit conversion / construction from std::vector
    TEArray(const std::vector<T> &vec) : m_Elements(vec) {}
    TEArray(std::vector<T> &&vec) : m_Elements(std::move(vec)) {}
    TEArray &operator=(const std::vector<T> &vec)
    {
        m_Elements = vec;
        return *this;
    }
    TEArray &operator=(std::vector<T> &&vec)
    {
        m_Elements = std::move(vec);
        return *this;
    }
    operator std::vector<T> &() { return m_Elements; }
    operator const std::vector<T> &() const { return m_Elements; }

    // Capacity & Size
    size_t Num() const { return m_Elements.size(); }
    size_t Size() const { return m_Elements.size(); }
    size_t size() const { return m_Elements.size(); }
    bool IsEmpty() const { return m_Elements.empty(); }
    bool empty() const { return m_Elements.empty(); }
    void Reserve(size_t capacity) { m_Elements.reserve(capacity); }
    void reserve(size_t capacity) { m_Elements.reserve(capacity); }
    void Resize(size_t newSize, const T &value = T()) { m_Elements.resize(newSize, value); }
    void resize(size_t newSize, const T &value = T()) { m_Elements.resize(newSize, value); }

    // Assignment & Modification
    void Assign(size_t count, const T &value) { m_Elements.assign(count, value); }
    template <typename InputIt> void Assign(InputIt first, InputIt last) { m_Elements.assign(first, last); }
    void Assign(std::initializer_list<T> ilist) { m_Elements.assign(ilist); }
    void assign(size_t count, const T &value) { m_Elements.assign(count, value); }
    template <typename InputIt> void assign(InputIt first, InputIt last) { m_Elements.assign(first, last); }
    void assign(std::initializer_list<T> ilist) { m_Elements.assign(ilist); }

    // Add & Insert
    void Add(const T &element) { m_Elements.push_back(element); }
    void Add(T &&element) { m_Elements.push_back(std::move(element)); }
    void push_back(const T &element) { m_Elements.push_back(element); }
    void push_back(T &&element) { m_Elements.push_back(std::move(element)); }
    void pop_back() { m_Elements.pop_back(); }

    template <typename... Args> reference Emplace(Args &&...args)
    {
        return m_Elements.emplace_back(std::forward<Args>(args)...);
    }

    template <typename... Args> reference emplace_back(Args &&...args)
    {
        return m_Elements.emplace_back(std::forward<Args>(args)...);
    }

    void Insert(size_t index, const T &element)
    {
        if (index <= m_Elements.size())
            m_Elements.insert(m_Elements.begin() + index, element);
    }

    iterator insert(const_iterator pos, const T &value) { return m_Elements.insert(pos, value); }
    iterator insert(const_iterator pos, T &&value) { return m_Elements.insert(pos, std::move(value)); }

    // Element Access
    reference operator[](size_t index) { return m_Elements[index]; }
    const_reference operator[](size_t index) const { return m_Elements[index]; }

    reference Get(size_t index) { return m_Elements.at(index); }
    const_reference Get(size_t index) const { return m_Elements.at(index); }
    reference at(size_t index) { return m_Elements.at(index); }
    const_reference at(size_t index) const { return m_Elements.at(index); }

    reference Front() { return m_Elements.front(); }
    const_reference Front() const { return m_Elements.front(); }
    reference front() { return m_Elements.front(); }
    const_reference front() const { return m_Elements.front(); }

    reference Back() { return m_Elements.back(); }
    const_reference Back() const { return m_Elements.back(); }
    reference back() { return m_Elements.back(); }
    const_reference back() const { return m_Elements.back(); }
    reference Last() { return m_Elements.back(); }
    const_reference Last() const { return m_Elements.back(); }
    reference last() { return m_Elements.back(); }
    const_reference last() const { return m_Elements.back(); }

    T *Data() { return m_Elements.data(); }
    const T *Data() const { return m_Elements.data(); }
    T *data() { return m_Elements.data(); }
    const T *data() const { return m_Elements.data(); }
    T *GetData() { return m_Elements.data(); }
    const T *GetData() const { return m_Elements.data(); }

    // Search & Equality
    bool Contains(const T &element) const
    {
        return std::find(m_Elements.begin(), m_Elements.end(), element) != m_Elements.end();
    }

    template <typename Predicate> bool ContainsBy(Predicate pred) const
    {
        return std::any_of(m_Elements.begin(), m_Elements.end(), pred);
    }

    int IndexOf(const T &element) const
    {
        auto it = std::find(m_Elements.begin(), m_Elements.end(), element);
        if (it != m_Elements.end())
            return static_cast<int>(std::distance(m_Elements.begin(), it));
        return -1;
    }

    template <typename Predicate> T *FindBy(Predicate pred)
    {
        auto it = std::find_if(m_Elements.begin(), m_Elements.end(), pred);
        return (it != m_Elements.end()) ? &(*it) : nullptr;
    }

    template <typename Predicate> const T *FindBy(Predicate pred) const
    {
        auto it = std::find_if(m_Elements.begin(), m_Elements.end(), pred);
        return (it != m_Elements.end()) ? &(*it) : nullptr;
    }

    // Removal
    bool Remove(const T &element)
    {
        auto it = std::find(m_Elements.begin(), m_Elements.end(), element);
        if (it != m_Elements.end())
        {
            m_Elements.erase(it);
            return true;
        }
        return false;
    }

    void RemoveAt(size_t index)
    {
        if (index < m_Elements.size())
            m_Elements.erase(m_Elements.begin() + index);
    }

    template <typename Predicate> size_t RemoveBy(Predicate pred)
    {
        auto oldSize = m_Elements.size();
        m_Elements.erase(std::remove_if(m_Elements.begin(), m_Elements.end(), pred), m_Elements.end());
        return oldSize - m_Elements.size();
    }

    iterator erase(const_iterator pos) { return m_Elements.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m_Elements.erase(first, last); }

    void Empty() { m_Elements.clear(); }
    void Clear() { m_Elements.clear(); }
    void clear() { m_Elements.clear(); }

    // Iterators
    iterator begin() { return m_Elements.begin(); }
    iterator end() { return m_Elements.end(); }
    const_iterator begin() const { return m_Elements.begin(); }
    const_iterator end() const { return m_Elements.end(); }
    const_iterator cbegin() const { return m_Elements.cbegin(); }
    const_iterator cend() const { return m_Elements.cend(); }

    reverse_iterator rbegin() { return m_Elements.rbegin(); }
    reverse_iterator rend() { return m_Elements.rend(); }
    const_reverse_iterator rbegin() const { return m_Elements.rbegin(); }
    const_reverse_iterator rend() const { return m_Elements.rend(); }
    const_reverse_iterator crbegin() const { return m_Elements.crbegin(); }
    const_reverse_iterator crend() const { return m_Elements.crend(); }

    // Operators
    bool operator==(const TEArray<T> &other) const { return m_Elements == other.m_Elements; }
    bool operator!=(const TEArray<T> &other) const { return m_Elements != other.m_Elements; }
    bool operator==(const std::vector<T> &other) const { return m_Elements == other; }
    bool operator!=(const std::vector<T> &other) const { return m_Elements != other; }
};

// ==========================================
// TEMap - Engine Hash Map Template
// ==========================================
template <typename KeyType, typename ValueType, typename Hasher = std::hash<KeyType>> class TEMap
{
private:
    std::unordered_map<KeyType, ValueType, Hasher> m_Map;

public:
    using key_type = KeyType;
    using mapped_type = ValueType;
    using value_type = std::pair<const KeyType, ValueType>;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hasher;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = typename std::unordered_map<KeyType, ValueType, Hasher>::iterator;
    using const_iterator = typename std::unordered_map<KeyType, ValueType, Hasher>::const_iterator;

    TEMap() = default;
    TEMap(std::initializer_list<value_type> initList) : m_Map(initList) {}

    // Implicit conversion / assignment with std::unordered_map
    TEMap(const std::unordered_map<KeyType, ValueType, Hasher> &other) : m_Map(other) {}
    TEMap(std::unordered_map<KeyType, ValueType, Hasher> &&other) : m_Map(std::move(other)) {}
    TEMap &operator=(const std::unordered_map<KeyType, ValueType, Hasher> &other)
    {
        m_Map = other;
        return *this;
    }
    TEMap &operator=(std::unordered_map<KeyType, ValueType, Hasher> &&other)
    {
        m_Map = std::move(other);
        return *this;
    }
    operator std::unordered_map<KeyType, ValueType, Hasher> &() { return m_Map; }
    operator const std::unordered_map<KeyType, ValueType, Hasher> &() const { return m_Map; }

    size_t Num() const { return m_Map.size(); }
    size_t Size() const { return m_Map.size(); }
    size_t size() const { return m_Map.size(); }
    bool IsEmpty() const { return m_Map.empty(); }
    bool empty() const { return m_Map.empty(); }

    void Add(const KeyType &key, const ValueType &value) { m_Map[key] = value; }
    void Add(const KeyType &key, ValueType &&value) { m_Map[key] = std::move(value); }

    ValueType &operator[](const KeyType &key) { return m_Map[key]; }
    ValueType &operator[](KeyType &&key) { return m_Map[std::move(key)]; }

    bool Contains(const KeyType &key) const { return m_Map.find(key) != m_Map.end(); }

    size_t count(const KeyType &key) const { return m_Map.count(key); }

    ValueType &at(const KeyType &key) { return m_Map.at(key); }

    const ValueType &at(const KeyType &key) const { return m_Map.at(key); }

    ValueType *Find(const KeyType &key)
    {
        auto it = m_Map.find(key);
        return (it != m_Map.end()) ? &(it->second) : nullptr;
    }

    const ValueType *Find(const KeyType &key) const
    {
        auto it = m_Map.find(key);
        return (it != m_Map.end()) ? &(it->second) : nullptr;
    }

    iterator find(const KeyType &key) { return m_Map.find(key); }
    const_iterator find(const KeyType &key) const { return m_Map.find(key); }

    bool Remove(const KeyType &key) { return m_Map.erase(key) > 0; }

    bool Erase(const KeyType &key) { return m_Map.erase(key) > 0; }

    size_t erase(const KeyType &key) { return m_Map.erase(key); }
    iterator erase(const_iterator pos) { return m_Map.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m_Map.erase(first, last); }

    void Empty() { m_Map.clear(); }
    void Clear() { m_Map.clear(); }
    void clear() { m_Map.clear(); }

    TEArray<KeyType> GetKeys() const
    {
        TEArray<KeyType> keys;
        keys.Reserve(m_Map.size());
        for (const auto &pair : m_Map)
            keys.Add(pair.first);
        return keys;
    }

    TEArray<ValueType> GetValues() const
    {
        TEArray<ValueType> values;
        values.Reserve(m_Map.size());
        for (const auto &pair : m_Map)
            values.Add(pair.second);
        return values;
    }

    iterator begin() { return m_Map.begin(); }
    iterator end() { return m_Map.end(); }
    const_iterator begin() const { return m_Map.begin(); }
    const_iterator end() const { return m_Map.end(); }
    const_iterator cbegin() const { return m_Map.cbegin(); }
    const_iterator cend() const { return m_Map.cend(); }
};

// ==========================================
// TESet - Engine Hash Set Template
// ==========================================
template <typename KeyType, typename Hasher = std::hash<KeyType>> class TESet
{
private:
    std::unordered_set<KeyType, Hasher> m_Set;

public:
    using key_type = KeyType;
    using value_type = KeyType;
    using size_type = size_t;
    using difference_type = ptrdiff_t;
    using hasher = Hasher;
    using reference = value_type &;
    using const_reference = const value_type &;
    using iterator = typename std::unordered_set<KeyType, Hasher>::iterator;
    using const_iterator = typename std::unordered_set<KeyType, Hasher>::const_iterator;

    TESet() = default;
    TESet(std::initializer_list<KeyType> initList) : m_Set(initList) {}

    TESet(const std::unordered_set<KeyType, Hasher> &other) : m_Set(other) {}
    TESet(std::unordered_set<KeyType, Hasher> &&other) : m_Set(std::move(other)) {}
    TESet &operator=(const std::unordered_set<KeyType, Hasher> &other)
    {
        m_Set = other;
        return *this;
    }
    TESet &operator=(std::unordered_set<KeyType, Hasher> &&other)
    {
        m_Set = std::move(other);
        return *this;
    }
    operator std::unordered_set<KeyType, Hasher> &() { return m_Set; }
    operator const std::unordered_set<KeyType, Hasher> &() const { return m_Set; }

    size_t Num() const { return m_Set.size(); }
    size_t Size() const { return m_Set.size(); }
    size_t size() const { return m_Set.size(); }
    bool IsEmpty() const { return m_Set.empty(); }
    bool empty() const { return m_Set.empty(); }

    void Add(const KeyType &key) { m_Set.insert(key); }
    void Add(KeyType &&key) { m_Set.insert(std::move(key)); }
    void insert(const KeyType &key) { m_Set.insert(key); }
    void insert(KeyType &&key) { m_Set.insert(std::move(key)); }

    bool Contains(const KeyType &key) const { return m_Set.find(key) != m_Set.end(); }
    size_t count(const KeyType &key) const { return m_Set.count(key); }
    iterator find(const KeyType &key) { return m_Set.find(key); }
    const_iterator find(const KeyType &key) const { return m_Set.find(key); }

    bool Remove(const KeyType &key) { return m_Set.erase(key) > 0; }
    size_t erase(const KeyType &key) { return m_Set.erase(key); }
    iterator erase(const_iterator pos) { return m_Set.erase(pos); }
    iterator erase(const_iterator first, const_iterator last) { return m_Set.erase(first, last); }

    void Empty() { m_Set.clear(); }
    void Clear() { m_Set.clear(); }
    void clear() { m_Set.clear(); }

    bool operator==(const TESet &other) const { return m_Set == other.m_Set; }
    bool operator!=(const TESet &other) const { return m_Set != other.m_Set; }

    iterator begin() { return m_Set.begin(); }
    iterator end() { return m_Set.end(); }
    const_iterator begin() const { return m_Set.begin(); }
    const_iterator end() const { return m_Set.end(); }
    const_iterator cbegin() const { return m_Set.cbegin(); }
    const_iterator cend() const { return m_Set.cend(); }
};

// ==========================================
// TEMatch (Pattern Matching over Algebraic Data Types / std::variant)
// ==========================================
template <typename Variant, typename... Handlers> decltype(auto) TEMatch(Variant &&v, Handlers &&...handlers)
{
    struct Overloaded : Handlers...
    {
        using Handlers::operator()...;
    };
    return std::visit(Overloaded{std::forward<Handlers>(handlers)...}, std::forward<Variant>(v));
}

// ==========================================
// Managed Memory Allocation & Lifetime Helpers
// ==========================================
namespace Memory
{
template <typename T, typename... Args> inline Scope<T> NewScope(Args &&...args)
{
    return CreateScope<T>(std::forward<Args>(args)...);
}

template <typename T, typename... Args> inline Ref<T> NewRef(Args &&...args)
{
    return CreateRef<T>(std::forward<Args>(args)...);
}
} // namespace Memory

// ==========================================
// GameplayUtils - Gameplay & Scene Utilities
// ==========================================
class TE_API GameplayUtils : public TFunctionLibrary
{
public:
    inline static const TEString StaticClassName = "GameplayUtils";

    // ── Container helpers ───────────────────────────────────────────────
    template <typename Container, typename Predicate>
    static auto FindFirst(Container &container, Predicate pred) -> decltype(container.Data())
    {
        for (size_t i = 0; i < container.Num(); ++i)
        {
            if (pred(container[i]))
                return &container[i];
        }
        return nullptr;
    }

    template <typename T> static bool Equals(const T &a, const T &b) { return a == b; }

    // ── Entity helpers ──────────────────────────────────────────────────
    static TEString GetEntityDisplayName(class EntityManager &mgr, Entity entity);
    static bool EntityContainsPoint(class EntityManager &mgr, Entity entity, const TEVector2 &worldPoint);
    static TEMatrix4 ResolveWorldTransform(class EntityManager &mgr, Entity entity, class TComponent *comp);
    static Entity PickEntity(Scene &scene, const TEVector2 &worldPoint);
    static TEVector2 ViewportPixelToWorld(TEVector2 pixelPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                          float cameraZoom);
    static TEVector2 WorldToViewportPixel(TEVector2 worldPos, TEVector2 viewportSize, TEVector2 cameraPos,
                                          float cameraZoom);

    // ── Spawning & UI helpers ───────────────────────────────────────────
    static Entity SpawnControllableGameObject(Scene &scene, const TEString &name = "ControllableObject",
                                              const TEVector2 &position = {0.0f, 0.0f});
    static TERef<class UIWidget> CreateWidget(const TEString &uiAssetPath);
};

#define TE_PLUGIN_ENABLED(PluginName)                                                                                  \
    (defined(TE_PLUGIN_##PluginName##_ENABLED) && (TE_PLUGIN_##PluginName##_ENABLED == 1))
#define TE_IS_PLUGIN_ENABLED(PluginName)                                                                               \
    (defined(TE_PLUGIN_##PluginName##_ENABLED) && (TE_PLUGIN_##PluginName##_ENABLED == 1))
