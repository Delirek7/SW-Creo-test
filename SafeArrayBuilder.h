#pragma once
#include <windows.h>
#include <comdef.h>

// This is a "Builder" tool. 
// Its job is to take a piece of C++ data and "pack" it into a COM Suitcase (SAFEARRAY).
class SafeArrayBuilder {
public:
    // A Generic tool to create an array of numbers (Doubles, Floats, etc.)
    // Returns a variant_t which will "OWN" the memory and clean it up.
    template <typename T>
    static variant_t Create(const T* data, long count, VARTYPE vt) {
        SAFEARRAYBOUND bounds = { (ULONG)count, 0 };
        
        // 1. Create the physical suitcase in Windows memory
        SAFEARRAY* psa = SafeArrayCreate(vt, 1, &bounds);
        
        if (psa) {
            void* pDest = nullptr;
            // 2. Lock the memory so we can write to it
            SafeArrayAccessData(psa, &pDest);
            
            // 3. High-speed bit-level copy (Fastest way to move data)
            memcpy(pDest, data, count * sizeof(T));
            
            // 4. Unlock the memory
            SafeArrayUnaccessData(psa);
        }

        // 5. Wrap it in a variant_t so C++ RAII takes over ownership
        variant_t result;
        result.vt = VT_ARRAY | vt;
        result.parray = psa;
        
        return result;
    }
};
