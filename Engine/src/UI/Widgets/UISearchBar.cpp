#include "UI/Widgets/UISearchBar.hpp"
#include "Core/PreRequisites.h"
#include "Utils/MathUtils.hpp"
#include "Utils/TimeGUI.hpp"
#include <algorithm>
#include <cctype>

int UISearchBar::ComputeLevenshteinDistance(const TEString &s1, const TEString &s2)
{
    size_t len1 = s1.Length();
    size_t len2 = s2.Length();

    if (len1 == 0)
        return (int)len2;
    if (len2 == 0)
        return (int)len1;

    TEArray<int> prevRow;
    TEArray<int> currRow;
    prevRow.Resize(len2 + 1);
    currRow.Resize(len2 + 1);

    for (size_t j = 0; j <= len2; ++j)
        prevRow[j] = (int)j;

    for (size_t i = 0; i < len1; ++i)
    {
        currRow[0] = (int)(i + 1);
        char c1 = (char)std::tolower((unsigned char)s1[i]);

        for (size_t j = 0; j < len2; ++j)
        {
            char c2 = (char)std::tolower((unsigned char)s2[j]);
            int cost = (c1 == c2) ? 0 : 1;
            currRow[j + 1] = std::min({currRow[j] + 1, prevRow[j + 1] + 1, prevRow[j] + cost});
        }
        prevRow = currRow;
    }

    return prevRow[len2];
}

bool UISearchBar::FuzzyMatch(const TEString &pattern, const TEString &target, float *outScore)
{
    if (pattern.empty())
    {
        if (outScore)
            *outScore = 1.0f;
        return true;
    }

    TEString p = pattern.ToLower();
    TEString t = target.ToLower();

    // 1. Exact Match
    if (t == p)
    {
        if (outScore)
            *outScore = 200.0f;
        return true;
    }

    // 2. Prefix Match
    if (t.StartsWith(p))
    {
        if (outScore)
            *outScore = 150.0f;
        return true;
    }

    // 3. Substring Match
    int subPos = t.Find(p);
    if (subPos != -1)
    {
        if (outScore)
            *outScore = 100.0f - (float)subPos;
        return true;
    }

    // 4. Acronym Match (e.g. "pfx" matches "ParticleFX")
    TEString acronym = "";
    bool nextIsAcronym = true;
    for (size_t i = 0; i < target.Length(); ++i)
    {
        char c = target[i];
        if (std::isupper((unsigned char)c) || nextIsAcronym)
        {
            if (std::isalnum((unsigned char)c))
            {
                acronym.Append((char)std::tolower((unsigned char)c));
                nextIsAcronym = false;
            }
        }
        if (c == ' ' || c == '_' || c == '-' || c == '.')
        {
            nextIsAcronym = true;
        }
    }

    if (!acronym.empty() && acronym.Contains(p))
    {
        if (outScore)
            *outScore = 80.0f;
        return true;
    }

    // 5. Typo-Tolerant Levenshtein Distance Match on individual words
    TEArray<TEString> words = t.Split(' ');
    for (const auto &word : words)
    {
        if (word.Length() >= 3 && p.Length() >= 3)
        {
            int dist = ComputeLevenshteinDistance(p, word);
            int maxAllowedDist = (p.Length() <= 4) ? 1 : 2;
            if (dist <= maxAllowedDist)
            {
                if (outScore)
                    *outScore = 60.0f - (float)dist * 15.0f;
                return true;
            }
        }
    }

    // Overall Levenshtein
    if (p.Length() >= 3 && t.Length() >= 3)
    {
        int dist = ComputeLevenshteinDistance(p, t);
        if (dist <= 2)
        {
            if (outScore)
                *outScore = 40.0f - (float)dist * 10.0f;
            return true;
        }
    }

    return false;
}

UISearchBar::UISearchBar(const TEString &placeholder, const TEString &id)
    : UIWidget(id), m_TextBox(placeholder, id + "_tb")
{
    m_TextBox.OnTextChanged = [this](const TEString &query)
    {
        if (OnQueryChanged)
            OnQueryChanged(query);
    };

    m_TextBox.OnEnterPressed = [this](const TEString &query)
    {
        if (m_SelectedPredictionIndex >= 0 && m_SelectedPredictionIndex < (int)m_ActivePredictions.Size())
        {
            SetQuery(m_ActivePredictions[m_SelectedPredictionIndex].Candidate);
            m_ShowPopup = false;
        }
        if (OnSearchSubmitted)
            OnSearchSubmitted(GetQuery());
    };
}

void UISearchBar::SetQuery(const TEString &query)
{
    m_TextBox.SetText(query);
    if (OnQueryChanged)
        OnQueryChanged(query);
}

void UISearchBar::Clear()
{
    m_TextBox.Clear();
    m_ActivePredictions.Clear();
    m_ShowPopup = false;
}

void UISearchBar::Focus() { m_TextBox.Focus(); }

bool UISearchBar::Matches(const TEString &text) const { return FuzzyMatch(m_TextBox.GetText(), text); }

void UISearchBar::DrawAutocompletePopup()
{
    if (m_ActivePredictions.empty())
        return;

    TEVector2 pos = TimeGUI::GetItemRectMin();
    TEVector2 size = TimeGUI::GetItemRectSize();
    pos.y += size.y + 2.0f;

    TimeGUI::SetNextWindowPos(pos);
    TimeGUI::SetNextWindowSize(TEVector2(size.x, 0.0f));

    TimeGUIWindowFlags popupFlags = TimeGUIWindowFlags_NoTitleBar | TimeGUIWindowFlags_NoResize |
                                    TimeGUIWindowFlags_NoMove | TimeGUIWindowFlags_NoSavedSettings |
                                    TimeGUIWindowFlags_AlwaysAutoResize;

    if (TimeGUI::Begin("##SearchPredictionsPopup", nullptr, popupFlags))
    {
        for (int i = 0; i < (int)m_ActivePredictions.Size(); ++i)
        {
            bool isSelected = (i == m_SelectedPredictionIndex);
            const auto &pred = m_ActivePredictions[i];

            if (TimeGUI::Selectable(pred.Candidate, isSelected))
            {
                SetQuery(pred.Candidate);
                m_ShowPopup = false;
            }

            if (TimeGUI::IsItemHovered())
            {
                m_SelectedPredictionIndex = i;
            }
        }
        TimeGUI::End();
    }
}

void UISearchBar::DrawSelf()
{
    m_TextBox.SetSize(m_Size);
    m_TextBox.Draw();

    // Clear Button (X)
    if (!GetQuery().empty())
    {
        TimeGUI::SameLine(0, 4.0f);
        if (TimeGUI::SmallButton("X"))
        {
            Clear();
        }
    }

    // Predictive Autocomplete Processing
    TEString query = GetQuery();
    if (!query.empty() && !m_Candidates.empty() && m_TextBox.IsFocused())
    {
        m_ActivePredictions.Clear();
        for (const auto &cand : m_Candidates)
        {
            float score = 0.0f;
            if (FuzzyMatch(query, cand, &score))
            {
                PredictiveMatchResult res;
                res.Candidate = cand;
                res.Score = score;
                res.IsDirectMatch = (cand.ToLower().Find(query.ToLower()) != -1);
                m_ActivePredictions.Add(res);
            }
        }

        // Sort predictions by relevance score descending
        std::sort(m_ActivePredictions.begin(), m_ActivePredictions.end(),
                  [](const PredictiveMatchResult &a, const PredictiveMatchResult &b) { return a.Score > b.Score; });

        // Limit to top 6 predictions
        if (m_ActivePredictions.Size() > 6)
        {
            m_ActivePredictions.Resize(6);
        }

        if (!m_ActivePredictions.empty())
        {
            DrawAutocompletePopup();
        }
    }
}
