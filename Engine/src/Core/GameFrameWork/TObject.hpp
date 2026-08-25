#pragma once
#include "Core/PreRequisites.h"
#include <atomic>
#include <cstdint>

TE_CLASS()
class TE_API TObject {
protected:
    static inline std::atomic<uint64_t> IDCounter = 1;

    TEPROPERTY()
    uint64_t ID = IDCounter++;

    TEPROPERTY()
    TEString Name;

    TEPROPERTY()
    TObject* Owner = nullptr;

    TEPROPERTY()
    bool bMarkedPendingDestroy = false;

    TEPROPERTY()
    bool bInitialized = false;

public:
    TObject(const TEString& name = "UnnamedObject") : Name(name) {}

    uint64_t GetID() const { return ID; }
    const TEString& GetName() const { return Name; }
    void SetName(const TEString& newName) { Name = newName; }

    TObject* GetOwner() const { return Owner; }
    void SetOwner(TObject* newOwner) { Owner = newOwner; }

    void OnInitialize() { bInitialized = true; }
    void Tick(float deltaTime) {}

    void MarkPendingDestroy() { bMarkedPendingDestroy = true; }
    bool IsMarkedPendingDestroy() const { return bMarkedPendingDestroy; }
    bool IsInitialized() const { return bInitialized; }

    void Destroy() {}

    inline static const TEString StaticClassName = "TObject";
};
