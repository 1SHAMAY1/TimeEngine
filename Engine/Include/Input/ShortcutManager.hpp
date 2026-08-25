#pragma once

#include "Core/KeyCodes.hpp"
#include "Core/PreRequisites.h"
#include "GameFrameWork/GameplayUtils.hpp"
#include <cstdint>
#include <functional>


enum class KeyModifier : uint8_t
{
    None  = 0,
    Ctrl  = 1 << 0,
    Shift = 1 << 1,
    Alt   = 1 << 2,
    Super = 1 << 3
};

inline KeyModifier operator|(KeyModifier a, KeyModifier b)
{
    return static_cast<KeyModifier>(static_cast<uint8_t>(a) | static_cast<uint8_t>(b));
}

inline KeyModifier operator&(KeyModifier a, KeyModifier b)
{
    return static_cast<KeyModifier>(static_cast<uint8_t>(a) & static_cast<uint8_t>(b));
}

inline KeyModifier &operator|=(KeyModifier &a, KeyModifier b)
{
    a = a | b;
    return a;
}

inline bool HasModifier(KeyModifier mask, KeyModifier flag)
{
    return (static_cast<uint8_t>(mask) & static_cast<uint8_t>(flag)) != 0;
}

struct TE_API Shortcut
{
    TEString ID;                               // Unique key, e.g. "Editor.SaveAll", "Editor.Undo"
    TEString DisplayName;                      // Human-readable title: "Save All", "Undo"
    TEString Category = "General";             // "File", "Edit", "Viewport", "PixelPaint", etc.
    TEString Context = "Global";               // Context/Scope name ("Global", "Editor", "PixelPaint")
    KeyCode Key = 0;                              // Primary key (e.g. Key::S, Key::Z)
    KeyModifier Modifiers = KeyModifier::None;    // Modifier mask (Ctrl, Shift, Alt)
    std::function<void()> Callback = nullptr;     // Optional direct callback
    std::function<bool()> IsContextActive = nullptr; // Optional predicate to check if context is active
    bool Enabled = true;                          // Master toggle
};

// Shortcut listener delegate: returns true if the event was consumed
using ShortcutListenerFn = std::function<bool(const TEString &shortcutId)>;

class TE_API ShortcutManager
{
public:
    // --- Registration & Management ---
    static bool RegisterShortcut(const Shortcut &shortcut);
    static bool UnregisterShortcut(const TEString &id);
    static bool RemapShortcut(const TEString &id, KeyCode newKey, KeyModifier newMods);
    static void Clear();

    // --- Querying & Inspection ---
    static bool HasShortcut(const TEString &id);
    static const Shortcut *GetShortcut(const TEString &id);
    static TEArray<Shortcut> GetAllShortcuts();
    static TEArray<Shortcut> GetShortcutsByCategory(const TEString &category);
    static TEArray<Shortcut> GetShortcutsByContext(const TEString &context);
    static TEString GetShortcutFormatString(const TEString &id);
    static TEString FormatKeyCombo(KeyCode key, KeyModifier mods);

    // --- Event Broadcast & Listener Bus ---
    static void AddListener(const TEString &listenerName, ShortcutListenerFn listener);
    static void RemoveListener(const TEString &listenerName);
    static bool BroadcastShortcut(const TEString &shortcutId);

    // --- Execution & Dispatching ---
    static bool TriggerShortcut(const TEString &id);
    static bool ProcessKeyPressed(KeyCode key, KeyModifier mods, const TEString &activeContext = "");
    static KeyModifier QueryCurrentModifiers();

    // --- Active Context Control ---
    static void SetActiveContext(const TEString &context);
    static const TEString &GetActiveContext();

private:
    static TEMap<TEString, Shortcut> &GetRegistry();
    static TEMap<TEString, ShortcutListenerFn> &GetListeners();
    static TEString s_ActiveContext;
};

// Declarative auto-registration template
struct TE_API ShortcutAutoRegister
{
    ShortcutAutoRegister(const char *id, const TEString& name, const TEString& category, const char *context,
                         KeyCode key, KeyModifier mods);
};

#define TE_REGISTER_SHORTCUT(ID, Name, Category, Context, Key, Mods) \
    static ShortcutAutoRegister s_AutoRegister_Shortcut_##ID(#ID, Name, Category, Context, Key, Mods);

