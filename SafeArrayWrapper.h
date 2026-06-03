#pragma once
#include <windows.h>
#include <atlbase.h> // For CComPtr

// This is a "Generic Template" class. 
// It works like a blueprint that can be used for any SolidWorks object type (T).
template <typename T>
class SafeArrayWrapper {
public:
    // Constructor: Takes the safearray data (psa) and finds its boundaries
    SafeArrayWrapper(SAFEARRAY* psa) : m_psa(psa) {
        if (m_psa) {
            SafeArrayGetLBound(m_psa, 1, &m_low);
            SafeArrayGetUBound(m_psa, 1, &m_high);
        } else {
            m_low = 0;
            m_high = -1;
        }
    }

    // Returns the total number of items in the array
    long Count() const {
        return (m_high - m_low + 1);
    }

    // Returns the item at a specific index, converted to the Smart Pointer type T
    T GetAt(long index) const {
        if (!m_psa) return nullptr;

        CComPtr<IUnknown> pUnk;
        // The address of the pointer (&pUnk.p) is where Windows will drop off the object
        SafeArrayGetElement(m_psa, &index, &pUnk.p);
        
        // T is a Smart Pointer type,
        // it automatically handles the cast and the AddRef
        return T(pUnk.p);
    }

    // Helpers for the loop
    long Lower() const { return m_low; }
    long Upper() const { return m_high; }

private:
    SAFEARRAY* m_psa;
    long m_low;
    long m_high;
};
