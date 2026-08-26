#pragma once

#include "UI/UIWidget.hpp"
#include "UI/Widgets/UITextBox.hpp"
#include <functional>

struct PredictiveMatchResult
{
    TEString Candidate;
    float Score = 0.0f; // Higher is better
    bool IsDirectMatch = false;
};

class TE_API UISearchBar : public UIWidget
{
public:
    UISearchBar(const TEString &placeholder = "Search...", const TEString &id = "##UISearchBar");
    virtual ~UISearchBar() = default;

    void DrawSelf() override;

    // Fuzzy & Predictive Typo-Tolerant Matching
    bool Matches(const TEString &text) const;
    static bool FuzzyMatch(const TEString &pattern, const TEString &target, float *outScore = nullptr);
    static int ComputeLevenshteinDistance(const TEString &s1, const TEString &s2);

    void Clear();
    void Focus();

    const TEString &GetQuery() const { return m_TextBox.GetText(); }
    void SetQuery(const TEString &query);

    // Candidates for Predictive Autocomplete
    void SetCandidates(const TEArray<TEString> &candidates) { m_Candidates = candidates; }
    void AddCandidate(const TEString &candidate) { m_Candidates.Add(candidate); }
    void ClearCandidates() { m_Candidates.Clear(); }

    std::function<void(const TEString &)> OnQueryChanged;
    std::function<void(const TEString &)> OnSearchSubmitted;

private:
    void DrawAutocompletePopup();

    UITextBox m_TextBox;
    TEArray<TEString> m_Candidates;
    TEArray<PredictiveMatchResult> m_ActivePredictions;
    int m_SelectedPredictionIndex = -1;
    bool m_ShowPopup = false;
};
