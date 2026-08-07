# Core Time Subsystem Architecture

The Core Time subsystem in TimeEngine provides frame timing, repeating tick counters (`Ticker`), and static delayed/looping timer callbacks (`Timer`).

> [!NOTE]
> In short, think of the **Time Subsystem** as the engine's internal clock and stopwatch: `Timer` allows scheduling tasks after $N$ seconds or frames (like delayed explosions or looping spawners), while `Ticker` acts as a metronome triggering periodic callbacks at fixed time rates.

---

## Component Overview

1. **[`TE::Timer`](file:///e:/TimeEngine/Engine/src/Core/Time/Timer.hpp)**: Static manager for scheduling delayed or looping callbacks based on elapsed time or frame counts.
2. **[`TE::Ticker`](file:///e:/TimeEngine/Engine/src/Core/Time/Ticker.hpp)**: Class instance managing interval-based tick callbacks with accumulators.

---

## Subsystem Architecture & Execution Rules

### 1. `TE::Timer` (Delayed & Frame-based Scheduling)

#### `Timer::Set(duration, callback, loop)`
- **When Used**: Schedule a callback after $X$ seconds (e.g. despawning a projectile after 3.0 seconds, or firing a repeating status effect every 1.0s).
- **When NOT to use**: Do NOT use for per-frame physics accumulation.

```cpp
// Schedule a looping timer every 2.0 seconds
auto ref = TE::Timer::Set(2.0f, []() {
    TE_CORE_INFO("Enemy spawned!");
}, true);
```

#### `Timer::SetFrames(frameCount, callback, loop)` & `Timer::NextFrame(callback)`
- **When Used**: Schedule work to execute after $N$ engine update frames or defer execution to the next frame.

```cpp
// Defer UI refresh to next frame after scene graph changes
TE::Timer::NextFrame([]() {
    RefreshHierarchyUI();
});
```

#### `Timer::Clear(ref)`
- **When Used**: Cancel an active timer before it triggers.

```cpp
TE::Timer::Clear(myTimerRef);
```

#### `Timer::Update(deltaTime)`
- **When Used**: Called internally once per frame inside `Application::Run()` update loop to progress all active timers.

---

### 2. `TE::Ticker` (Metronome / Interval Accumulator)

#### `Ticker::Update(deltaTime)`
- **When Used**: Progresses accumulator `m_Elapsed` and fires `m_Callback(deltaTime, m_Total)` when `m_Elapsed >= m_Rate`.

```cpp
// Create ticker firing 10 times per second (rate = 0.1s)
TE::Ticker ticker([](float& dt, float total) {
    // Fixed update logic
}, 0.1f);

// Update inside layer loop
ticker.Update(dt);
```
