#pragma once
#include "Core/PreRequisites.h"
class TObject;

TE_CLASS()
class TE_API TComponent {
protected:
    TEPROPERTY()
    TObject* Owner = nullptr;

    TEPROPERTY()
    bool bMarkedPendingDestroy = false;

    TEPROPERTY()
    bool bInitialized = false;

public:
    void SetOwner(TObject* newOwner) { Owner = newOwner; }
    TObject* GetOwner() const { return Owner; }

    void OnInitialize() { bInitialized = true; }
    void OnAttach() {}
    void OnDetach() {}
    void Tick(float deltaTime) {}

    void MarkPendingDestroy() { bMarkedPendingDestroy = true; }
    bool IsMarkedPendingDestroy() const { return bMarkedPendingDestroy; }
    bool IsInitialized() const { return bInitialized; }

    static constexpr const char* StaticClassName = "TComponent";
};

