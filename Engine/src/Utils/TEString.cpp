#include "Utils/TEString.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include "Utils/MathUtils.hpp"
#include <cctype>
#include <iomanip>
#include <regex>

// ==========================================
// TELocalizationManager Implementation
// ==========================================

TELocalizationManager::TELocalizationManager() : m_CurrentCulture("en-US") {}

TELocalizationManager &TELocalizationManager::Get()
{
    static TELocalizationManager s_Instance;
    return s_Instance;
}

void TELocalizationManager::SetCurrentCulture(const std::string &culture)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_CurrentCulture = culture;
}

std::string TELocalizationManager::GetCurrentCulture() const
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    return m_CurrentCulture;
}

void TELocalizationManager::RegisterStringTable(const std::string &culture, const std::string &ns,
                                                const std::string &key, const std::string &translation)
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Tables[culture][ns][key] = translation;
}

std::string TELocalizationManager::GetLocalizedString(const std::string &ns, const std::string &key,
                                                      const std::string &defaultVal) const
{
    std::lock_guard<std::mutex> lock(m_Mutex);

    // Check current culture table
    auto cultureIt = m_Tables.find(m_CurrentCulture);
    if (cultureIt != m_Tables.end())
    {
        auto nsIt = cultureIt->second.find(ns);
        if (nsIt != cultureIt->second.end())
        {
            auto keyIt = nsIt->second.find(key);
            if (keyIt != nsIt->second.end())
            {
                return keyIt->second;
            }
        }
    }

    // Fallback to "en-US" if different
    if (m_CurrentCulture != "en-US")
    {
        auto fallbackCultureIt = m_Tables.find("en-US");
        if (fallbackCultureIt != m_Tables.end())
        {
            auto nsIt = fallbackCultureIt->second.find(ns);
            if (nsIt != fallbackCultureIt->second.end())
            {
                auto keyIt = nsIt->second.find(key);
                if (keyIt != nsIt->second.end())
                {
                    return keyIt->second;
                }
            }
        }
    }

    return defaultVal;
}

void TELocalizationManager::ClearStringTables()
{
    std::lock_guard<std::mutex> lock(m_Mutex);
    m_Tables.clear();
}

// ==========================================
// TEString Implementation
// ==========================================

const TEString TEString::None = TEString("");

// 64-bit FNV-1a Hash Implementation
static inline uint64_t ComputeFNV1a64(const char *data, size_t length)
{
    constexpr uint64_t FNV_offset_basis = 14695981039346656037ULL;
    constexpr uint64_t FNV_prime = 1099511628211ULL;

    uint64_t hash = FNV_offset_basis;
    for (size_t i = 0; i < length; ++i)
    {
        hash ^= static_cast<uint64_t>(static_cast<unsigned char>(data[i]));
        hash *= FNV_prime;
    }
    return hash;
}

void TEString::ComputeHash() { m_Hash = ComputeFNV1a64(m_Data.data(), m_Data.length()); }

// Constructors
TEString::TEString() : m_Data(""), m_Hash(ComputeFNV1a64("", 0)) {}

TEString::TEString(const char *str) : m_Data(str ? str : "") { ComputeHash(); }

TEString::TEString(const char *str, size_t length) : m_Data(str ? str : "", length) { ComputeHash(); }

TEString::TEString(const wchar_t *wstr) : TEString(wstr, wstr ? wcslen(wstr) : 0) {}

TEString::TEString(const wchar_t *wstr, size_t length)
{
    if (!wstr || length == 0)
    {
        m_Data = "";
    }
    else
    {
#ifdef TE_PLATFORM_WINDOWS
        int utf8Len = WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(length), NULL, 0, NULL, NULL);
        if (utf8Len > 0)
        {
            m_Data.resize(utf8Len);
            WideCharToMultiByte(CP_UTF8, 0, wstr, static_cast<int>(length), &m_Data[0], utf8Len, NULL, NULL);
        }
        else
        {
            m_Data = "";
        }
#else
        std::string result;
        result.resize(length * 4);
        size_t converted = wcstombs(&result[0], wstr, result.size());
        if (converted != static_cast<size_t>(-1))
        {
            result.resize(converted);
            m_Data = result;
        }
        else
        {
            m_Data = "";
        }
#endif
    }
    ComputeHash();
}

TEString TEString::FromWide(const wchar_t *wstr) { return TEString(wstr); }

TEString::TEString(const std::string &str) : m_Data(str) { ComputeHash(); }

TEString::TEString(std::string &&str) noexcept : m_Data(std::move(str)) { ComputeHash(); }

TEString::TEString(std::string_view sv) : m_Data(sv) { ComputeHash(); }

TEString::TEString(const TELocalizedMeta &meta)
    : m_Namespace(meta.Namespace), m_Key(meta.Key), m_SourceText(meta.SourceText)
{
    m_Data = TELocalizationManager::Get().GetLocalizedString(m_Namespace, m_Key, m_SourceText);
    ComputeHash();
}

TEString::TEString(const TEString &other)
    : m_Data(other.m_Data), m_Hash(other.m_Hash), m_Namespace(other.m_Namespace), m_Key(other.m_Key),
      m_SourceText(other.m_SourceText)
{
}

TEString::TEString(TEString &&other) noexcept
    : m_Data(std::move(other.m_Data)), m_Hash(other.m_Hash), m_Namespace(std::move(other.m_Namespace)),
      m_Key(std::move(other.m_Key)), m_SourceText(std::move(other.m_SourceText))
{
    other.m_Hash = ComputeFNV1a64("", 0);
}

// Assignment Operators
TEString &TEString::operator=(const TEString &other)
{
    if (this != &other)
    {
        m_Data = other.m_Data;
        m_Hash = other.m_Hash;
        m_Namespace = other.m_Namespace;
        m_Key = other.m_Key;
        m_SourceText = other.m_SourceText;
    }
    return *this;
}

TEString &TEString::operator=(TEString &&other) noexcept
{
    if (this != &other)
    {
        m_Data = std::move(other.m_Data);
        m_Hash = other.m_Hash;
        m_Namespace = std::move(other.m_Namespace);
        m_Key = std::move(other.m_Key);
        m_SourceText = std::move(other.m_SourceText);
        other.m_Hash = ComputeFNV1a64("", 0);
    }
    return *this;
}

TEString &TEString::operator=(const char *str)
{
    m_Data = (str ? str : "");
    m_Namespace.clear();
    m_Key.clear();
    m_SourceText.clear();
    ComputeHash();
    return *this;
}

TEString &TEString::operator=(const std::string &str)
{
    m_Data = str;
    m_Namespace.clear();
    m_Key.clear();
    m_SourceText.clear();
    ComputeHash();
    return *this;
}

TEString &TEString::operator=(std::string_view sv)
{
    m_Data = sv;
    m_Namespace.clear();
    m_Key.clear();
    m_SourceText.clear();
    ComputeHash();
    return *this;
}

void TEString::Clear()
{
    m_Data.clear();
    m_Namespace.clear();
    m_Key.clear();
    m_SourceText.clear();
    ComputeHash();
}

void TEString::Shrink() { m_Data.shrink_to_fit(); }

void TEString::AutoCompact(size_t thresholdRatio)
{
    size_t cap = m_Data.capacity();
    size_t len = m_Data.length();
    // Only compact if capacity is excessively bloated (> 256 bytes) and more than thresholdRatio times the current
    // length
    if (cap > 256 && (len == 0 || cap >= len * thresholdRatio))
    {
        m_Data.shrink_to_fit();
    }
}

void TEString::SyncFromBuffer()
{
    if (!m_Data.empty() || m_Data.capacity() > 0)
    {
        size_t actualLen = strlen(m_Data.data());
        m_Data.resize(actualLen);
        RecomputeHashOnMutate();
    }
}

// Search & Substrings
static inline bool CaseInsensitiveEqual(char a, char b)
{
    return std::tolower(static_cast<unsigned char>(a)) == std::tolower(static_cast<unsigned char>(b));
}

bool TEString::Contains(const TEString &sub, ESearchCase searchCase) const { return Find(sub, searchCase) != -1; }

bool TEString::Contains(const char *sub, ESearchCase searchCase) const
{
    if (!sub)
        return false;
    return Find(TEString(sub), searchCase) != -1;
}

bool TEString::StartsWith(const TEString &prefix, ESearchCase searchCase) const
{
    if (prefix.Len() > Len())
        return false;

    if (searchCase == ESearchCase::CaseSensitive)
    {
        return m_Data.compare(0, prefix.Len(), prefix.m_Data) == 0;
    }
    else
    {
        return std::equal(prefix.m_Data.begin(), prefix.m_Data.end(), m_Data.begin(), CaseInsensitiveEqual);
    }
}

bool TEString::StartsWith(const char *prefix, ESearchCase searchCase) const
{
    if (!prefix)
        return false;
    return StartsWith(TEString(prefix), searchCase);
}

bool TEString::EndsWith(const TEString &suffix, ESearchCase searchCase) const
{
    if (suffix.Len() > Len())
        return false;

    size_t startPos = Len() - suffix.Len();
    if (searchCase == ESearchCase::CaseSensitive)
    {
        return m_Data.compare(startPos, suffix.Len(), suffix.m_Data) == 0;
    }
    else
    {
        return std::equal(suffix.m_Data.begin(), suffix.m_Data.end(), m_Data.begin() + startPos, CaseInsensitiveEqual);
    }
}

bool TEString::EndsWith(const char *suffix, ESearchCase searchCase) const
{
    if (!suffix)
        return false;
    return EndsWith(TEString(suffix), searchCase);
}

int TEString::Find(const TEString &sub, ESearchCase searchCase, ESearchDir searchDir, int startIdx) const
{
    if (sub.IsEmpty())
        return 0;
    if (startIdx < 0 || static_cast<size_t>(startIdx) >= m_Data.length())
        return -1;

    if (searchCase == ESearchCase::CaseSensitive)
    {
        if (searchDir == ESearchDir::FromStart)
        {
            size_t pos = m_Data.find(sub.m_Data, static_cast<size_t>(startIdx));
            return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
        }
        else
        {
            size_t pos = m_Data.rfind(sub.m_Data, static_cast<size_t>(startIdx));
            return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
        }
    }
    else
    {
        auto it = std::search(m_Data.begin() + startIdx, m_Data.end(), sub.m_Data.begin(), sub.m_Data.end(),
                              CaseInsensitiveEqual);
        if (it != m_Data.end())
        {
            return static_cast<int>(std::distance(m_Data.begin(), it));
        }
        return -1;
    }
}

int TEString::FindLast(const TEString &sub, ESearchCase searchCase) const
{
    if (m_Data.empty() || sub.IsEmpty())
        return -1;

    if (searchCase == ESearchCase::CaseSensitive)
    {
        size_t pos = m_Data.rfind(sub.m_Data);
        return (pos == std::string::npos) ? -1 : static_cast<int>(pos);
    }
    else
    {
        auto it =
            std::find_end(m_Data.begin(), m_Data.end(), sub.m_Data.begin(), sub.m_Data.end(), CaseInsensitiveEqual);
        if (it != m_Data.end())
        {
            return static_cast<int>(std::distance(m_Data.begin(), it));
        }
        return -1;
    }
}

TEString TEString::Left(size_t count) const
{
    if (count >= m_Data.length())
        return *this;
    return TEString(m_Data.substr(0, count));
}

TEString TEString::Right(size_t count) const
{
    if (count >= m_Data.length())
        return *this;
    return TEString(m_Data.substr(m_Data.length() - count));
}

TEString TEString::Mid(size_t start, size_t count) const
{
    if (start >= m_Data.length())
        return TEString();
    return TEString(m_Data.substr(start, count));
}

// Transformations & Mutators
TEString TEString::ToUpper() const
{
    std::string result = m_Data;
    for (char &c : result)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    return TEString(std::move(result));
}

TEString TEString::ToLower() const
{
    std::string result = m_Data;
    for (char &c : result)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    return TEString(std::move(result));
}

void TEString::ToUpperInline()
{
    for (char &c : m_Data)
        c = static_cast<char>(std::toupper(static_cast<unsigned char>(c)));
    ComputeHash();
}

void TEString::ToLowerInline()
{
    for (char &c : m_Data)
        c = static_cast<char>(std::tolower(static_cast<unsigned char>(c)));
    ComputeHash();
}

TEString TEString::TrimStart() const
{
    size_t start = m_Data.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return TEString();
    return TEString(m_Data.substr(start));
}

TEString TEString::TrimEnd() const
{
    size_t end = m_Data.find_last_not_of(" \t\n\r\f\v");
    if (end == std::string::npos)
        return TEString();
    return TEString(m_Data.substr(0, end + 1));
}

TEString TEString::Trim() const
{
    size_t start = m_Data.find_first_not_of(" \t\n\r\f\v");
    if (start == std::string::npos)
        return TEString();
    size_t end = m_Data.find_last_not_of(" \t\n\r\f\v");
    return TEString(m_Data.substr(start, end - start + 1));
}

void TEString::TrimStartInline() { *this = TrimStart(); }

void TEString::TrimEndInline() { *this = TrimEnd(); }

void TEString::TrimInline() { *this = Trim(); }

TEString TEString::Replace(const TEString &from, const TEString &to, ESearchCase searchCase) const
{
    if (from.IsEmpty())
        return *this;

    std::string result;
    size_t lastPos = 0;

    if (searchCase == ESearchCase::CaseSensitive)
    {
        size_t findPos = m_Data.find(from.m_Data, lastPos);
        while (findPos != std::string::npos)
        {
            result.append(m_Data, lastPos, findPos - lastPos);
            result.append(to.m_Data);
            lastPos = findPos + from.Len();
            findPos = m_Data.find(from.m_Data, lastPos);
        }
        result.append(m_Data, lastPos, m_Data.length() - lastPos);
    }
    else
    {
        int findPos = Find(from, ESearchCase::IgnoreCase, ESearchDir::FromStart, static_cast<int>(lastPos));
        while (findPos != -1)
        {
            result.append(m_Data, lastPos, static_cast<size_t>(findPos) - lastPos);
            result.append(to.m_Data);
            lastPos = static_cast<size_t>(findPos) + from.Len();
            findPos = Find(from, ESearchCase::IgnoreCase, ESearchDir::FromStart, static_cast<int>(lastPos));
        }
        result.append(m_Data, lastPos, m_Data.length() - lastPos);
    }

    return TEString(std::move(result));
}

void TEString::ReplaceInline(const TEString &from, const TEString &to, ESearchCase searchCase)
{
    *this = Replace(from, to, searchCase);
}

TEString TEString::Reverse() const
{
    std::string result = m_Data;
    std::reverse(result.begin(), result.end());
    return TEString(std::move(result));
}

void TEString::ReverseInline()
{
    std::reverse(m_Data.begin(), m_Data.end());
    ComputeHash();
}

void TEString::Append(const TEString &other)
{
    m_Data.append(other.m_Data);
    ComputeHash();
}

void TEString::Append(const char *str)
{
    if (str)
    {
        m_Data.append(str);
        ComputeHash();
    }
}

void TEString::Append(char ch)
{
    m_Data.push_back(ch);
    ComputeHash();
}

// Split & Join
TEArray<TEString> TEString::Split(const TEString &delimiter) const
{
    TEArray<TEString> result;
    if (delimiter.IsEmpty())
    {
        result.push_back(*this);
        return result;
    }

    size_t start = 0;
    size_t end = m_Data.find(delimiter.m_Data);
    while (end != std::string::npos)
    {
        result.emplace_back(m_Data.substr(start, end - start));
        start = end + delimiter.Len();
        end = m_Data.find(delimiter.m_Data, start);
    }
    result.emplace_back(m_Data.substr(start));
    return result;
}

TEArray<TEString> TEString::Split(char delimiter) const { return Split(TEString(std::string(1, delimiter))); }

TEString TEString::Join(const TEArray<TEString> &segments, const TEString &delimiter)
{
    if (segments.empty())
        return TEString();

    std::string result;
    for (size_t i = 0; i < segments.size(); ++i)
    {
        result.append(segments[i].m_Data);
        if (i + 1 < segments.size())
            result.append(delimiter.m_Data);
    }
    return TEString(std::move(result));
}

// Conversions
bool TEString::ToBool() const
{
    if (Equals("true", ESearchCase::IgnoreCase) || Equals("1") || Equals("yes", ESearchCase::IgnoreCase))
        return true;
    return false;
}

int TEString::ToInt() const
{
    try
    {
        return std::stoi(m_Data);
    }
    catch (...)
    {
        return 0;
    }
}

int64_t TEString::ToInt64() const
{
    try
    {
        return std::stoll(m_Data);
    }
    catch (...)
    {
        return 0;
    }
}

float TEString::ToFloat() const
{
    try
    {
        return std::stof(m_Data);
    }
    catch (...)
    {
        return 0.0f;
    }
}

double TEString::ToDouble() const
{
    try
    {
        return std::stod(m_Data);
    }
    catch (...)
    {
        return 0.0;
    }
}

TEString TEString::FromBool(bool val) { return val ? "true" : "false"; }

TEString TEString::FromInt(int val) { return std::to_string(val); }

TEString TEString::FromInt64(int64_t val) { return std::to_string(val); }

TEString TEString::FromFloat(float val, int precision)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << val;
    return ss.str();
}

TEString TEString::FromDouble(double val, int precision)
{
    std::ostringstream ss;
    ss << std::fixed << std::setprecision(precision) << val;
    return ss.str();
}

TEString TEString::FromVector(const TEVector &v) { return TEString::Printf("X=%.3f Y=%.3f Z=%.3f", v.x, v.y, v.z); }

TEString TEString::FromVector2(const TEVector2 &v) { return TEString::Printf("X=%.3f Y=%.3f", v.x, v.y); }

TEString TEString::FromVector4(const TEVector4 &v)
{
    return TEString::Printf("X=%.3f Y=%.3f Z=%.3f W=%.3f", v.x, v.y, v.z, v.w);
}

// Printf
TEString TEString::Printf(const char *fmt, ...)
{
    if (!fmt)
        return TEString();

    va_list args;
    va_start(args, fmt);
    va_list argsCopy;
    va_copy(argsCopy, args);

    int size = vsnprintf(nullptr, 0, fmt, argsCopy);
    va_end(argsCopy);

    if (size <= 0)
    {
        va_end(args);
        return TEString();
    }

    std::string buffer;
    buffer.resize(static_cast<size_t>(size));
    vsnprintf(&buffer[0], static_cast<size_t>(size + 1), fmt, args);
    va_end(args);

    return TEString(std::move(buffer));
}

// Localization Subsystem
TEString TEString::FromTable(const std::string &ns, const std::string &key, const std::string &defaultText)
{
    TELocalizedMeta meta;
    meta.Namespace = ns;
    meta.Key = key;
    meta.SourceText = defaultText;
    return TEString(meta);
}

TEString TEString::AsCultureInvariant(const std::string &str) { return TEString(str); }

TEString TEString::FormatText(const TEString &pattern, const std::unordered_map<std::string, TEString> &args)
{
    std::string result = pattern.m_Data;
    for (const auto &pair : args)
    {
        std::string token = "{" + pair.first + "}";
        size_t pos = result.find(token);
        while (pos != std::string::npos)
        {
            result.replace(pos, token.length(), pair.second.m_Data);
            pos = result.find(token, pos + pair.second.Len());
        }
    }
    return TEString(std::move(result));
}

TEString TEString::ResolveLocalized() const
{
    if (IsLocalized())
    {
        std::string translated = TELocalizationManager::Get().GetLocalizedString(m_Namespace, m_Key, m_SourceText);
        TEString resolved(translated);
        resolved.m_Namespace = m_Namespace;
        resolved.m_Key = m_Key;
        resolved.m_SourceText = m_SourceText;
        return resolved;
    }
    return *this;
}

// Comparisons
bool TEString::Equals(const TEString &other, ESearchCase searchCase) const
{
    if (searchCase == ESearchCase::CaseSensitive)
    {
        if (m_Hash != other.m_Hash)
            return false;
        return m_Data == other.m_Data;
    }
    else
    {
        if (m_Data.length() != other.m_Data.length())
            return false;
        return std::equal(m_Data.begin(), m_Data.end(), other.m_Data.begin(), CaseInsensitiveEqual);
    }
}

int TEString::Compare(const TEString &other, ESearchCase searchCase) const
{
    if (searchCase == ESearchCase::CaseSensitive)
    {
        return m_Data.compare(other.m_Data);
    }
    else
    {
        size_t minLen = std::min(m_Data.length(), other.m_Data.length());
        for (size_t i = 0; i < minLen; ++i)
        {
            int c1 = std::tolower(static_cast<unsigned char>(m_Data[i]));
            int c2 = std::tolower(static_cast<unsigned char>(other.m_Data[i]));
            if (c1 != c2)
                return c1 - c2;
        }
        if (m_Data.length() < other.m_Data.length())
            return -1;
        if (m_Data.length() > other.m_Data.length())
            return 1;
        return 0;
    }
}

// Concatenations
TEString TEString::operator+(const TEString &rhs) const
{
    TEString res(*this);
    res.Append(rhs);
    return res;
}

TEString TEString::operator+(const char *rhs) const
{
    TEString res(*this);
    res.Append(rhs);
    return res;
}

TEString TEString::operator+(const std::string &rhs) const
{
    TEString res(*this);
    res.Append(rhs.c_str());
    return res;
}

TEString TEString::operator+(char rhs) const
{
    TEString res(*this);
    res.Append(rhs);
    return res;
}

TEString &TEString::operator+=(const TEString &rhs)
{
    Append(rhs);
    return *this;
}

TEString &TEString::operator+=(const char *rhs)
{
    Append(rhs);
    return *this;
}

TEString &TEString::operator+=(const std::string &rhs)
{
    Append(rhs.c_str());
    return *this;
}

TEString &TEString::operator+=(char rhs)
{
    Append(rhs);
    return *this;
}

// ── Path Operations Implementation ──────────────────────────────────────
TEString TEString::operator/(const TEString &rhs) const
{
    if (IsEmpty())
        return rhs;
    if (rhs.IsEmpty())
        return *this;

    std::string lhsStr = m_Data;
    std::string rhsStr = rhs.ToStdString();

    bool lhsHasSlash = (lhsStr.back() == '/' || lhsStr.back() == '\\');
    bool rhsHasSlash = (rhsStr.front() == '/' || rhsStr.front() == '\\');

    if (lhsHasSlash && rhsHasSlash)
        return TEString(lhsStr + rhsStr.substr(1));
    else if (!lhsHasSlash && !rhsHasSlash)
        return TEString(lhsStr + "/" + rhsStr);
    else
        return TEString(lhsStr + rhsStr);
}

TEString TEString::operator/(const char *rhs) const { return *this / TEString(rhs ? rhs : ""); }

TEString &TEString::operator/=(const TEString &rhs)
{
    *this = *this / rhs;
    return *this;
}

TEString &TEString::operator/=(const char *rhs)
{
    *this = *this / TEString(rhs ? rhs : "");
    return *this;
}

TEString TEString::GetParentPath() const
{
    if (IsEmpty())
        return TEString("");

    std::string path = m_Data;
    // Strip trailing slash if present
    while (path.length() > 1 && (path.back() == '/' || path.back() == '\\'))
    {
        path.pop_back();
    }

    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos)
        return TEString("");
    if (lastSlash == 0)
        return TEString("/");

    return TEString(path.substr(0, lastSlash));
}

TEString TEString::GetFilename() const
{
    if (IsEmpty())
        return TEString("");

    std::string path = m_Data;
    while (path.length() > 1 && (path.back() == '/' || path.back() == '\\'))
    {
        path.pop_back();
    }

    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string::npos)
        return TEString(path);

    return TEString(path.substr(lastSlash + 1));
}

TEString TEString::GetStem() const
{
    TEString filename = GetFilename();
    if (filename.IsEmpty())
        return TEString("");

    std::string fn = filename.ToStdString();
    size_t dot = fn.find_last_of('.');
    if (dot == std::string::npos || dot == 0)
        return filename;

    return TEString(fn.substr(0, dot));
}

TEString TEString::GetExtension() const
{
    TEString filename = GetFilename();
    if (filename.IsEmpty())
        return TEString("");

    std::string fn = filename.ToStdString();
    size_t dot = fn.find_last_of('.');
    if (dot == std::string::npos || dot == 0)
        return TEString("");

    return TEString(fn.substr(dot));
}

bool TEString::HasExtension() const { return !GetExtension().IsEmpty(); }

TEString TEString::ReplaceExtension(const TEString &newExt) const
{
    TEString parent = GetParentPath();
    TEString stem = GetStem();
    TEString ext = newExt;

    if (!ext.IsEmpty() && !ext.StartsWith("."))
        ext = TEString(".") + ext;

    TEString newFilename = stem + ext;
    if (parent.IsEmpty())
        return newFilename;

    return parent / newFilename;
}

bool TEString::IsAbsolute() const
{
    if (IsEmpty())
        return false;
#ifdef TE_PLATFORM_WINDOWS
    if (m_Data.length() >= 2 && m_Data[1] == ':')
        return true;
    if (m_Data.length() >= 2 && m_Data[0] == '\\' && m_Data[1] == '\\')
        return true;
#endif
    return m_Data[0] == '/' || m_Data[0] == '\\';
}

bool TEString::IsRelative() const { return !IsAbsolute(); }

TEString TEString::LexicallyNormal() const
{
    if (IsEmpty())
        return TEString("");

    std::string result = m_Data;
    for (char &c : result)
    {
        if (c == '\\')
            c = '/';
    }

    return TEString(result);
}
