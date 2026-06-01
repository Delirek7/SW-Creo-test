#pragma once
#include <windows.h>
#include <comdef.h>
#include <iostream>

// Import SolidWorks (We need this in every wrapper file that talks to SW)
#import "C:\\Program Files\\SOLIDWORKS Corp\\SOLIDWORKS\\sldworks.tlb" no_namespace, named_guids

class SwApp {
public:
    // Constructor: Starts or connects to SolidWorks
    SwApp();

    // Destructor: Automatically releases the connection
    ~SwApp() = default;

    // Returns the raw SolidWorks pointer (needed by other classes)
    ISldWorksPtr Get() const { return m_swApp; }

    // Helper to check if we are actually connected
    bool IsValid() const { return m_swApp != nullptr; }

private:
    ISldWorksPtr m_swApp;
};
