#include "Input/ShortcutManager.hpp"
#include "Core/Log.h"
#include "Core/PreRequisites.h"
#include "Input/Input.hpp"
#include <algorithm>
#include <sstream>

#include <mutex>

static std::mutex s_ShortcutMutex;
static std::mutex s_ListenerMutex;
static bool s_IsShutdown = false;

TEString ShortcutManager::s_ActiveContext = "Global";

static TERef<TEMap<TEString, Shortcut>> s_RegistryRef;
static TERef<TEMap<TEString, ShortcutListenerFn>> s_ListenersRef;

TEMap<TEString, Shortcut> &ShortcutManager::GetRegistry()
{
    if (!s_RegistryRef)
        s_RegistryRef = CreateRef<TEMap<TEString, Shortcut>>();
    return *s_RegistryRef;
}

TEMap<TEString, ShortcutListenerFn> &ShortcutManager::GetListeners()
{
    if (!s_ListenersRef)
        s_ListenersRef = CreateRef<TEMap<TEString, ShortcutListenerFn>>();
    return *s_ListenersRef;
}

void ShortcutManager::Shutdown()
{
    std::lock_guard<std::mutex> lock(s_ListenerMutex);
    s_IsShutdown = true;
    if (s_ListenersRef)
        s_ListenersRef->clear();
    TE_CORE_INFO("[ShortcutManager] Shutdown complete. Listeners cleared.");
}

ShortcutAutoRegister::ShortcutAutoRegister(const char *id, const TEString &name, const TEString &category,
                                           const char *context, KeyCode key, KeyModifier mods)
{
    Shortcut sc;
    sc.ID = id ? id : "";
    sc.DisplayName = name.empty() ? "" : name;
    sc.Category = category.empty() ? "General" : category;
    sc.Context = context ? context : "Global";
    sc.Key = key;
    sc.Modifiers = mods;
    sc.Enabled = true;
    ShortcutManager::RegisterShortcut(sc);
}

bool ShortcutManager::RegisterShortcut(const Shortcut &shortcut)
{
    if (shortcut.ID.empty())
    {
        TE_CORE_ERROR("ShortcutManager::RegisterShortcut failed: Empty shortcut ID provided.");
        return false;
    }

    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    auto &reg = GetRegistry();
    reg[shortcut.ID] = shortcut;
    return true;
}

bool ShortcutManager::UnregisterShortcut(const TEString &id)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    auto &reg = GetRegistry();
    auto it = reg.find(id);
    if (it != reg.end())
    {
        reg.erase(it);
        return true;
    }
    return false;
}

bool ShortcutManager::RemapShortcut(const TEString &id, KeyCode newKey, KeyModifier newMods)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    auto &reg = GetRegistry();
    auto it = reg.find(id);
    if (it != reg.end())
    {
        it->second.Key = newKey;
        it->second.Modifiers = newMods;
        TE_CORE_INFO("ShortcutManager: Remapped '{0}' to {1}", id, FormatKeyCombo(newKey, newMods));
        return true;
    }
    TE_CORE_WARN("ShortcutManager::RemapShortcut: Shortcut ID '{0}' not found.", id);
    return false;
}

void ShortcutManager::Clear()
{
    {
        std::lock_guard<std::mutex> lock(s_ShortcutMutex);
        GetRegistry().clear();
    }
    {
        std::lock_guard<std::mutex> lock(s_ListenerMutex);
        GetListeners().clear();
    }
}

bool ShortcutManager::HasShortcut(const TEString &id)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    const auto &reg = GetRegistry();
    return reg.find(id) != reg.end();
}

const Shortcut *ShortcutManager::GetShortcut(const TEString &id)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    const auto &reg = GetRegistry();
    auto it = reg.find(id);
    if (it != reg.end())
        return &it->second;
    return nullptr;
}

TEArray<Shortcut> ShortcutManager::GetAllShortcuts()
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    TEArray<Shortcut> result;
    const auto &reg = GetRegistry();
    result.Reserve(reg.size());
    for (const auto &pair : reg)
    {
        result.Add(pair.second);
    }
    return result;
}

TEArray<Shortcut> ShortcutManager::GetShortcutsByCategory(const TEString &category)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    TEArray<Shortcut> result;
    const auto &reg = GetRegistry();
    for (const auto &pair : reg)
    {
        if (pair.second.Category == category)
            result.Add(pair.second);
    }
    return result;
}

TEArray<Shortcut> ShortcutManager::GetShortcutsByContext(const TEString &context)
{
    std::lock_guard<std::mutex> lock(s_ShortcutMutex);
    TEArray<Shortcut> result;
    const auto &reg = GetRegistry();
    for (const auto &pair : reg)
    {
        if (pair.second.Context == context || pair.second.Context == "Global")
            result.Add(pair.second);
    }
    return result;
}

void ShortcutManager::AddListener(const TEString &listenerName, ShortcutListenerFn listener)
{
    if (listenerName.empty() || !listener)
        return;
    std::lock_guard<std::mutex> lock(s_ListenerMutex);
    GetListeners()[listenerName] = listener;
}

void ShortcutManager::RemoveListener(const TEString &listenerName)
{
    if (listenerName.empty() || s_IsShutdown)
        return;
    std::lock_guard<std::mutex> lock(s_ListenerMutex);
    if (s_IsShutdown)
        return;
    auto &listeners = GetListeners();
    auto it = listeners.find(listenerName);
    if (it != listeners.end())
    {
        listeners.erase(it);
    }
}

bool ShortcutManager::BroadcastShortcut(const TEString &shortcutId)
{
    if (shortcutId.empty() || s_IsShutdown)
        return false;

    TEArray<ShortcutListenerFn> activeListeners;
    {
        std::lock_guard<std::mutex> lock(s_ListenerMutex);
        if (s_IsShutdown)
            return false;
        for (const auto &pair : GetListeners())
        {
            if (pair.second)
                activeListeners.push_back(pair.second);
        }
    }

    bool handled = false;
    for (const auto &listener : activeListeners)
    {
        if (listener && listener(shortcutId))
        {
            handled = true;
            break;
        }
    }
    return handled;
}

static TEString KeyCodeToString(KeyCode key)
{
    switch (key)
    {
    case Key::Space:
        return "Space";
    case Key::Apostrophe:
        return "'";
    case Key::Comma:
        return ",";
    case Key::Minus:
        return "-";
    case Key::Period:
        return ".";
    case Key::Slash:
        return "/";
    case Key::Semicolon:
        return ";";
    case Key::Equal:
        return "=";
    case Key::LeftBracket:
        return "[";
    case Key::Backslash:
        return "\\";
    case Key::RightBracket:
        return "]";
    case Key::GraveAccent:
        return "`";
    case Key::Escape:
        return "Esc";
    case Key::Enter:
        return "Enter";
    case Key::Tab:
        return "Tab";
    case Key::Backspace:
        return "Backspace";
    case Key::Insert:
        return "Insert";
    case Key::Delete:
        return "Delete";
    case Key::Right:
        return "Right";
    case Key::Left:
        return "Left";
    case Key::Down:
        return "Down";
    case Key::Up:
        return "Up";
    case Key::PageUp:
        return "PageUp";
    case Key::PageDown:
        return "PageDown";
    case Key::Home:
        return "Home";
    case Key::End:
        return "End";
    case Key::CapsLock:
        return "CapsLock";
    case Key::ScrollLock:
        return "ScrollLock";
    case Key::NumLock:
        return "NumLock";
    case Key::PrintScreen:
        return "PrintScreen";
    case Key::Pause:
        return "Pause";
    case Key::F1:
        return "F1";
    case Key::F2:
        return "F2";
    case Key::F3:
        return "F3";
    case Key::F4:
        return "F4";
    case Key::F5:
        return "F5";
    case Key::F6:
        return "F6";
    case Key::F7:
        return "F7";
    case Key::F8:
        return "F8";
    case Key::F9:
        return "F9";
    case Key::F10:
        return "F10";
    case Key::F11:
        return "F11";
    case Key::F12:
        return "F12";
    default:
        if (key >= Key::D0 && key <= Key::D9)
        {
            return TEString::FromInt(key - Key::D0);
        }
        if (key >= Key::A && key <= Key::Z)
        {
            char c = (char)('A' + (key - Key::A));
            return TEString(&c, 1);
        }
        return "Key_" + TEString::FromInt(key);
    }
}

TEString ShortcutManager::FormatKeyCombo(KeyCode key, KeyModifier mods)
{
    if (key == 0)
        return "None";

    TEString res = "";
    if (HasModifier(mods, KeyModifier::Ctrl))
        res += "Ctrl+";
    if (HasModifier(mods, KeyModifier::Alt))
        res += "Alt+";
    if (HasModifier(mods, KeyModifier::Shift))
        res += "Shift+";
    if (HasModifier(mods, KeyModifier::Super))
        res += "Super+";

    res += KeyCodeToString(key);
    return res;
}

TEString ShortcutManager::GetShortcutFormatString(const TEString &id)
{
    const auto *sc = GetShortcut(id);
    if (!sc)
        return "";
    return FormatKeyCombo(sc->Key, sc->Modifiers);
}

bool ShortcutManager::TriggerShortcut(const TEString &id)
{
    bool handled = false;
    const auto *sc = GetShortcut(id);
    if (sc && sc->Enabled)
    {
        if (sc->Callback)
        {
            sc->Callback();
            handled = true;
        }
        if (BroadcastShortcut(id))
        {
            handled = true;
        }
    }
    return handled;
}

KeyModifier ShortcutManager::QueryCurrentModifiers()
{
    KeyModifier mods = KeyModifier::None;
    if (Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl))
        mods |= KeyModifier::Ctrl;
    if (Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift))
        mods |= KeyModifier::Shift;
    if (Input::IsKeyPressed(Key::LeftAlt) || Input::IsKeyPressed(Key::RightAlt))
        mods |= KeyModifier::Alt;
    if (Input::IsKeyPressed(Key::LeftSuper) || Input::IsKeyPressed(Key::RightSuper))
        mods |= KeyModifier::Super;
    return mods;
}

bool ShortcutManager::ProcessKeyPressed(KeyCode key, KeyModifier mods, const TEString &activeContext)
{
    if (key == 0)
        return false;

    // Modifiers alone shouldn't trigger shortcuts directly
    if (key == Key::LeftControl || key == Key::RightControl || key == Key::LeftShift || key == Key::RightShift ||
        key == Key::LeftAlt || key == Key::RightAlt || key == Key::LeftSuper || key == Key::RightSuper)
    {
        return false;
    }

    TEString contextToMatch = activeContext.empty() ? s_ActiveContext : activeContext;

    auto &reg = GetRegistry();
    for (auto &pair : reg)
    {
        auto &sc = pair.second;
        if (!sc.Enabled)
            continue;

        if (sc.Key != key || sc.Modifiers != mods)
            continue;

        // Context check: Match exact context, or "Global"
        if (sc.Context != "Global" && !sc.Context.empty())
        {
            if (sc.Context != contextToMatch)
            {
                if (!sc.IsContextActive || !sc.IsContextActive())
                    continue;
            }
        }

        if (sc.IsContextActive && !sc.IsContextActive())
            continue;

        bool handled = false;
        if (sc.Callback)
        {
            sc.Callback();
            handled = true;
        }

        // Broadcast to listeners
        if (BroadcastShortcut(sc.ID))
        {
            handled = true;
        }

        if (handled)
            return true;
    }

    return false;
}

void ShortcutManager::SetActiveContext(const TEString &context) { s_ActiveContext = context; }

const TEString &ShortcutManager::GetActiveContext() { return s_ActiveContext; }

// ---------------------------------------------------------------------------
// Static Persistent Core Shortcuts Registration
// ---------------------------------------------------------------------------
TE_REGISTER_SHORTCUT(Editor_SaveAll, "Save All Content", "File", "Global", Key::S, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_Undo, "Undo", "Edit", "Global", Key::Z, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_Redo, "Redo", "Edit", "Global", Key::Y, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_SelectAll, "Select All", "Edit", "Global", Key::A, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_Cut, "Cut", "Edit", "Global", Key::X, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_Copy, "Copy Selected", "Edit", "Global", Key::C, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_Paste, "Paste Selected", "Edit", "Global", Key::V, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_DeleteSelected, "Delete Selected", "Edit", "Global", Key::Delete, KeyModifier::None);
TE_REGISTER_SHORTCUT(Editor_Duplicate, "Duplicate Selected", "Edit", "Global", Key::D, KeyModifier::Ctrl);
TE_REGISTER_SHORTCUT(Editor_ClearSelection, "Clear Selection", "Selection", "Global", Key::Escape, KeyModifier::None);
TE_REGISTER_SHORTCUT(Editor_GizmoNone, "Gizmo: None", "Viewport", "Editor", Key::Q, KeyModifier::None);
TE_REGISTER_SHORTCUT(Editor_GizmoTranslate, "Gizmo: Translate", "Viewport", "Editor", Key::W, KeyModifier::None);
TE_REGISTER_SHORTCUT(Editor_GizmoRotate, "Gizmo: Rotate", "Viewport", "Editor", Key::E, KeyModifier::None);
TE_REGISTER_SHORTCUT(Editor_GizmoScale, "Gizmo: Scale", "Viewport", "Editor", Key::R, KeyModifier::None);
