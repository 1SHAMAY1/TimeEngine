#pragma once
#include "Core/PreRequisites.h"
#include <string>
#include <atomic>
#include <cstdint>

TE_CLASS()
class TE_API TObject {
protected:
    static inline std::atomic<uint64_t> IDCounter = 1;

    TEPROPERTY()
    uint64_t ID = IDCounter++;

    TEPROPERTY()
    std::string Name;

    TEPROPERTY()
    TObject* Owner = nullptr;

    TEPROPERTY()
    bool bMarkedPendingDestroy = false;

    TEPROPERTY()
    bool bInitialized = false;

public:
    TObject(const std::string& name = "UnnamedObject") : Name(name) {}

    uint64_t GetID() const { return ID; }
    const std::string& GetName() const { return Name; }
    void SetName(const std::string& newName) { Name = newName; }

    TObject* GetOwner() const { return Owner; }
    void SetOwner(TObject* newOwner) { Owner = newOwner; }

    void OnInitialize() { bInitialized = true; }
    void Tick(float deltaTime) {}

    void MarkPendingDestroy() { bMarkedPendingDestroy = true; }
    bool IsMarkedPendingDestroy() const { return bMarkedPendingDestroy; }
    bool IsInitialized() const { return bInitialized; }

    void Destroy() {}

    static constexpr const char* StaticClassName = "TObject";
};
