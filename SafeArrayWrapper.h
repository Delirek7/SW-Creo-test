#pragma once
#include <windows.h>
#include <atlbase.h> // For CComPtr
#include <type_traits> // For static_assert and type traits
#include <vector>

// This is a "Generic Template" class. 
// It works like a blueprint that can be used for any SolidWorks object type (T).
template <typename T>
class SafeArrayWrapper {
public:
    // We restrict T to be a "Smart Pointer" type.
    // std::is_class ensures T is a class/struct (like IBody2Ptr), not a raw pointer or double.
    // std::is_convertible ensures it can be initialized from a COM pointer.
    static_assert(std::is_class_v<T>, "SafeArrayWrapper: T must be a Smart Pointer class (e.g., IBody2Ptr).");

    SafeArrayWrapper(SAFEARRAY* psa) : m_psa(psa) {
        if (m_psa) {
            SafeArrayGetLBound(m_psa, 1, &m_low);
            SafeArrayGetUBound(m_psa, 1, &m_high);
        } else {
            m_low = 0;
            m_high = -1;
        }
    }

    // Returns all elements in a standard C++ vector for easy iteration
    std::vector<T> GetAllElements() const {
        std::vector<T> result;
        if (!m_psa || Count() <= 0) return result;

        result.reserve(Count()); // Optimization: Prevent multiple reallocations
        for (long i = m_low; i <= m_high; i++) {
            result.push_back(GetAt(i));
        }
        return result;
    }

    long Count() const {
        return (m_high - m_low + 1);
    }

    T GetAt(long index) const {
        if (!m_psa) return nullptr;

        CComPtr<IUnknown> pUnk;
        SafeArrayGetElement(m_psa, &index, &pUnk.p);
        return T(pUnk.p);
    }

    long Lower() const { return m_low; }
    long Upper() const { return m_high; }

private:
    SAFEARRAY* m_psa;
    long m_low;
    long m_high;
};
