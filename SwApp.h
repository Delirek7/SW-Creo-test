#pragma once
#include <windows.h>
#include <comdef.h>
#include <iostream>
#include <stdexcept>

// Import SolidWorks
#import "C:\\Program Files\\SOLIDWORKS Corp\\SOLIDWORKS\\sldworks.tlb" no_namespace, named_guids

class SwApp {
public:
    // Constructor: GUARANTEES a connection or throws an exception.
    SwApp();

    ~SwApp() = default;

    // Guaranteed to return a valid pointer because of the constructor check.
    ISldWorksPtr Get() const { return m_swApp; }

private:
    ISldWorksPtr m_swApp;
};
