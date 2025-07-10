#pragma once
#include "Core/GameFrameWork/TFunctionLibrary.hpp"
#include "TObject.hpp"
#include <vector>
#include <memory>
#include <type_traits>

TE_CLASS()
class StandardGameLibrary : public TFunctionLibrary {
protected:
    static inline std::vector<std::unique_ptr<TObject>> g_ObjectPool;

public:
    template<typename T, typename... Args>
    static T& CreateObject(Args&&... args) {
        static_assert(std::is_base_of<TObject, T>::value, "T must inherit from TObject");

        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        T& ref = *obj;
        g_ObjectPool.emplace_back(std::move(obj));
        return ref;
    }

    template<typename T, typename... Args>
    static T& CreateObjectWithOwner(TObject* owner, Args&&... args) {
        static_assert(std::is_base_of<TObject, T>::value, "T must inherit from TObject");

        auto obj = std::make_unique<T>(std::forward<Args>(args)...);
        obj->SetOwner(owner);
        T& ref = *obj;
        g_ObjectPool.emplace_back(std::move(obj));
        return ref;
    }

    static void ClearAllObjects() {
        g_ObjectPool.clear();
    }

    static constexpr const char* StaticClassName = "StandardGameLibrary";
};
