#pragma once

#ifndef TE_API
#include "Core/PreRequisites.h"
#endif
#include <algorithm>
#include <cstdarg>
#include <cstdint>
#include <mutex>
#include <sstream>
#include <string>
#include <string_view>
#include <unordered_map>
#include <vector>

// Forward declaration of math types and containers
struct TEVector;
struct TEVector2;
struct TEVector4;
template <typename T> class TEArray;

enum class ESearchCase
{
    CaseSensitive,
    IgnoreCase
};

enum class ESearchDir
{
    FromStart,
    FromEnd
};

// ==========================================
// Localization Metadata
// ==========================================
struct TELocalizedMeta
{
    std::string Namespace;
    std::string Key;
    std::string SourceText;
};

// ==========================================
// TELocalizationManager - Engine Localization Registry
// ==========================================
class TE_API TELocalizationManager
{
public:
    static TELocalizationManager &Get();

    void SetCurrentCulture(const std::string &culture);
    std::string GetCurrentCulture() const;

    void RegisterStringTable(const std::string &culture, const std::string &ns, const std::string &key,
                             const std::string &translation);
    std::string GetLocalizedString(const std::string &ns, const std::string &key, const std::string &defaultVal) const;
    void ClearStringTables();

private:
    TELocalizationManager();
    ~TELocalizationManager() = default;

    mutable std::mutex m_Mutex;
    std::string m_CurrentCulture = "en-US";
    // Map: Culture -> (Namespace -> (Key -> Translation))
    std::unordered_map<std::string, std::unordered_map<std::string, std::unordered_map<std::string, std::string>>>
        m_Tables;
};

// ==========================================
// TEString - Unified Engine String Type
// ==========================================
class TE_API TEString
{
public:
    // Static constant for empty / none state
    static const TEString None;

    // Constructors
    TEString();
    TEString(const char *str);
    TEString(const char *str, size_t length);
    TEString(const wchar_t *wstr);
    TEString(const wchar_t *wstr, size_t length);
    TEString(const std::string &str);
    TEString(std::string &&str) noexcept;
    TEString(std::string_view sv);
    TEString(const TELocalizedMeta &meta);
    TEString(const TEString &other);
    TEString(TEString &&other) noexcept;

    static TEString FromWide(const wchar_t *wstr);

    ~TEString() = default;

    // Assignment Operators
    TEString &operator=(const TEString &other);
    TEString &operator=(TEString &&other) noexcept;
    TEString &operator=(const char *str);
    TEString &operator=(const std::string &str);
    TEString &operator=(std::string_view sv);

    // ── Fast Hash & Identifier (FName-style) ───────────────────────────
    uint64_t GetHash() const { return m_Hash; }
    uint32_t GetID() const { return static_cast<uint32_t>(m_Hash & 0xFFFFFFFF); }
    bool IsNone() const { return m_Data.empty(); }

    static constexpr size_t npos = static_cast<size_t>(-1);

    // ── Size & Capacity (FString-style + STL-style) ────────────────────
    size_t Len() const { return m_Data.length(); }
    size_t Length() const { return m_Data.length(); }
    size_t Num() const { return m_Data.length(); }
    size_t Size() const { return m_Data.length(); }
    size_t size() const { return m_Data.length(); }
    size_t length() const { return m_Data.length(); }
    bool IsEmpty() const { return m_Data.empty(); }
    bool empty() const { return m_Data.empty(); }
    void Reserve(size_t capacity) { m_Data.reserve(capacity); }
    void reserve(size_t capacity) { m_Data.reserve(capacity); }
    void Empty() { Clear(); }
    void Clear();
    size_t Capacity() const { return m_Data.capacity(); }
    size_t capacity() const { return m_Data.capacity(); }
    void Shrink();
    void AutoCompact(size_t thresholdRatio = 4);
    void SyncFromBuffer();

    // ── Element Access & STL Compatibility ──────────────────────────────
    const char *c_str() const { return m_Data.c_str(); }
    const char *Data() const { return m_Data.data(); }
    const char *data() const { return m_Data.data(); }
    char *Data() { return const_cast<char *>(m_Data.data()); }
    char *data() { return const_cast<char *>(m_Data.data()); }
    const std::string &ToStdString() const { return m_Data; }
    std::string_view ToStringView() const { return std::string_view(m_Data); }

    char operator[](size_t index) const { return m_Data[index]; }
    char &operator[](size_t index)
    {
        RecomputeHashOnMutate();
        return m_Data[index];
    }

    operator const std::string &() const { return m_Data; }
    explicit operator std::string_view() const { return std::string_view(m_Data); }
    explicit operator const char *() const { return m_Data.c_str(); }

    auto begin() { return m_Data.begin(); }
    auto end() { return m_Data.end(); }
    auto begin() const { return m_Data.begin(); }
    auto end() const { return m_Data.end(); }
    auto cbegin() const { return m_Data.cbegin(); }
    auto cend() const { return m_Data.cend(); }

    size_t find_first_not_of(const char *s, size_t pos = 0) const { return m_Data.find_first_not_of(s, pos); }
    size_t find_last_not_of(const char *s, size_t pos = npos) const { return m_Data.find_last_not_of(s, pos); }
    size_t find(const std::string &str, size_t pos = 0) const { return m_Data.find(str, pos); }
    size_t find(char c, size_t pos = 0) const { return m_Data.find(c, pos); }
    size_t rfind(const std::string &str, size_t pos = npos) const { return m_Data.rfind(str, pos); }
    size_t rfind(char c, size_t pos = npos) const { return m_Data.rfind(c, pos); }
    TEString substr(size_t pos = 0, size_t len = npos) const { return Mid(pos, len); }

    // ── Search & Substrings ─────────────────────────────────────────────
    bool Contains(const TEString &sub, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    bool Contains(const char *sub, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    bool StartsWith(const TEString &prefix, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    bool StartsWith(const char *prefix, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    bool EndsWith(const TEString &suffix, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    bool EndsWith(const char *suffix, ESearchCase searchCase = ESearchCase::CaseSensitive) const;

    int Find(const TEString &sub, ESearchCase searchCase = ESearchCase::CaseSensitive,
             ESearchDir searchDir = ESearchDir::FromStart, int startIdx = 0) const;
    int FindLast(const TEString &sub, ESearchCase searchCase = ESearchCase::CaseSensitive) const;

    TEString Left(size_t count) const;
    TEString Right(size_t count) const;
    TEString Mid(size_t start, size_t count = std::string::npos) const;
    TEString Substr(size_t start, size_t count = std::string::npos) const { return Mid(start, count); }

    // ── Transformations & Mutators ──────────────────────────────────────
    TEString ToUpper() const;
    TEString ToLower() const;
    void ToUpperInline();
    void ToLowerInline();

    TEString TrimStart() const;
    TEString TrimEnd() const;
    TEString Trim() const;
    void TrimStartInline();
    void TrimEndInline();
    void TrimInline();

    TEString Replace(const TEString &from, const TEString &to,
                     ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    void ReplaceInline(const TEString &from, const TEString &to, ESearchCase searchCase = ESearchCase::CaseSensitive);
    TEString Reverse() const;
    void ReverseInline();

    void Append(const TEString &other);
    void Append(const char *str);
    void Append(char ch);

    // ── Split & Join ────────────────────────────────────────────────────
    TEArray<TEString> Split(const TEString &delimiter) const;
    TEArray<TEString> Split(char delimiter) const;
    static TEString Join(const TEArray<TEString> &segments, const TEString &delimiter);

    // ── Parsing & Numerical Conversions ─────────────────────────────────
    bool ToBool() const;
    int ToInt() const;
    int64_t ToInt64() const;
    float ToFloat() const;
    double ToDouble() const;

    static TEString FromBool(bool val);
    static TEString FromInt(int val);
    static TEString FromInt64(int64_t val);
    static TEString FromFloat(float val, int precision = 2);
    static TEString FromDouble(double val, int precision = 4);
    static TEString FromVector(const TEVector &v);
    static TEString FromVector2(const TEVector2 &v);
    static TEString FromVector4(const TEVector4 &v);

    // ── Formatting (Printf & Token Format) ───────────────────────────────
    static TEString Printf(const char *fmt, ...);

    template <typename T>
    static auto FormatArg(T &&arg) -> decltype(auto)
    {
        using Decayed = std::decay_t<T>;
        if constexpr (std::is_same_v<Decayed, TEString>)
            return arg.c_str();
        else if constexpr (std::is_same_v<Decayed, std::string>)
            return arg.c_str();
        else if constexpr (std::is_same_v<Decayed, std::string_view>)
            return arg.data();
        else
            return std::forward<T>(arg);
    }

    template <typename... Args> static TEString Format(const char *fmt, Args &&...args)
    {
        return Printf(fmt, FormatArg(std::forward<Args>(args))...);
    }

    template <typename... Args> static TEString Format(const TEString &fmt, Args &&...args)
    {
        return Printf(fmt.c_str(), FormatArg(std::forward<Args>(args))...);
    }

    // ── Localization Subsystem (FText-style) ────────────────────────────
    static TEString FromTable(const std::string &ns, const std::string &key, const std::string &defaultText);
    static TEString AsCultureInvariant(const std::string &str);
    static TEString FormatText(const TEString &pattern, const std::unordered_map<std::string, TEString> &args);

    bool IsLocalized() const { return !m_Namespace.empty() && !m_Key.empty(); }
    const std::string &GetNamespace() const { return m_Namespace; }
    const std::string &GetKey() const { return m_Key; }
    const std::string &GetSourceText() const { return m_SourceText; }

    /// Resolves the string dynamically based on the current active culture if localized
    TEString ResolveLocalized() const;

    // ── Comparison Operators ────────────────────────────────────────────
    bool Equals(const TEString &other, ESearchCase searchCase = ESearchCase::CaseSensitive) const;
    int Compare(const TEString &other, ESearchCase searchCase = ESearchCase::CaseSensitive) const;

    bool operator<(const TEString &other) const { return m_Data < other.m_Data; }
    bool operator<=(const TEString &other) const { return m_Data <= other.m_Data; }
    bool operator>(const TEString &other) const { return m_Data > other.m_Data; }
    bool operator>=(const TEString &other) const { return m_Data >= other.m_Data; }

    // ── Concatenation Operators ─────────────────────────────────────────
    TEString operator+(const TEString &rhs) const;
    TEString operator+(const char *rhs) const;
    TEString operator+(const std::string &rhs) const;
    TEString operator+(char rhs) const;

    TEString &operator+=(const TEString &rhs);
    TEString &operator+=(const char *rhs);
    TEString &operator+=(const std::string &rhs);
    TEString &operator+=(char rhs);

    // ── Path Operations (Filesystem Abstraction) ────────────────────────
    TEString operator/(const TEString &rhs) const;
    TEString operator/(const char *rhs) const;
    TEString &operator/=(const TEString &rhs);
    TEString &operator/=(const char *rhs);

    TEString GetParentPath() const;
    bool HasParentPath() const { return !GetParentPath().IsEmpty(); }
    TEString GetFilename() const;
    bool HasFilename() const { return !GetFilename().IsEmpty(); }
    TEString GetStem() const;
    TEString GetExtension() const;
    bool HasExtension() const;
    TEString ReplaceExtension(const TEString &newExt) const;
    bool IsAbsolute() const;
    bool IsRelative() const;
    TEString LexicallyNormal() const;

private:
    void ComputeHash();
    void RecomputeHashOnMutate() { m_Hash = 0; }

private:
    std::string m_Data;
    uint64_t m_Hash = 0;

    // Localization context (optional)
    std::string m_Namespace;
    std::string m_Key;
    std::string m_SourceText;
};

// Standalone Non-Member Comparison Operators
inline bool operator==(const TEString &lhs, const TEString &rhs)
{
    if (lhs.GetHash() != rhs.GetHash())
        return false;
    return lhs.ToStdString() == rhs.ToStdString();
}

inline bool operator==(const TEString &lhs, const char *rhs) { return lhs.ToStdString() == (rhs ? rhs : ""); }

inline bool operator==(const char *lhs, const TEString &rhs) { return rhs == lhs; }

inline bool operator==(const TEString &lhs, const std::string &rhs) { return lhs.ToStdString() == rhs; }

inline bool operator==(const std::string &lhs, const TEString &rhs) { return lhs == rhs.ToStdString(); }

// Global Concatenation Overloads
inline TEString operator+(const char *lhs, const TEString &rhs)
{
    TEString result(lhs);
    result.Append(rhs);
    return result;
}

inline TEString operator+(const std::string &lhs, const TEString &rhs)
{
    TEString result(lhs);
    result.Append(rhs);
    return result;
}

// Stream insertion operator
inline std::ostream &operator<<(std::ostream &os, const TEString &str)
{
    os << str.c_str();
    return os;
}

// Hash specialization for standard library containers
namespace std
{
template <> struct hash<TEString>
{
    size_t operator()(const TEString &str) const noexcept { return static_cast<size_t>(str.GetHash()); }
};
} // namespace std

// Localization Macros
#define TE_LOC(Namespace, Key, DefaultText) TEString::FromTable(Namespace, Key, DefaultText)
#define TE_LOCTEXT(Key, DefaultText) TEString::FromTable("", Key, DefaultText)
