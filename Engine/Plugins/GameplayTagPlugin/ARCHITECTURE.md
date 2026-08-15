# GameplayTagPlugin Architecture

The `GameplayTagPlugin` module provides a standalone, opt-in hierarchical tag system for TimeEngine entities and systems.

> [!NOTE]
> Hierarchical gameplay tags allow coarse or granular queries against entity properties. For example, querying `has_tag("Character.Enemy")` matches entities with tags like `Character.Enemy.Boss.Dragon` or `Character.Enemy.Minion`.

---

## Core Classes & Concepts

### 1. `GameplayTag` (`src/GameplayTag.hpp`)
- Represents a single hierarchical dot-delimited string tag (e.g. `Character.Enemy.Boss`).
- `MatchesTag(queryTag)`: Evaluates parent-child relationship (`Character.Enemy.Boss` matches query `Character.Enemy`).
- `MatchesExact(other)`: Evaluates exact string equivalence.
- `GetParentTag()`: Extracts parent level tag (e.g. `Character.Enemy` for `Character.Enemy.Boss`).

### 2. `GameplayTagContainer` (`src/GameplayTagContainer.hpp`)
- Collection container for entity gameplay tags.
- Methods:
  - `AddTag(tag)` / `RemoveTag(tag)`
  - `HasTag(tagQuery)`: Hierarchical match check.
  - `HasTagExact(tagQuery)`: Exact match check.
  - `HasAllTags(container)` / `HasAnyTags(container)`

### 3. `GameplayTagManager` (`src/GameplayTagManager.hpp`)
- Singleton plugin registry (`GameplayTagManager::Get()`) tracking registered tags and implicit ancestor nodes.

### 4. `GameplayTagComponent` (`src/GameplayTagComponent.hpp`)
- ECS component (`TComponent`) wrapping a `GameplayTagContainer` for entity integration. Exposes `has_tag(tag)` and `HasTag(tag)` query methods.

---

## Usage Example

```cpp
// Querying entity tags in C++ or script callbacks
if (otherEntity.GetComponent<GameplayTagComponent>()->has_tag("Character.Enemy"))
{
    health -= 25.0f;
}

if (otherEntity.GetComponent<GameplayTagComponent>()->has_tag("Status.Invincible"))
{
    return;
}
```
