#include "SwApp.h"

SwApp::SwApp() {
    // Constructing connection
    try {                                                           // Is SolidWorks already open?
        if (FAILED(m_swApp.GetActiveObject(__uuidof(SldWorks)))) {  // __uuidof search for app ID in Win Registry
            if (FAILED(m_swApp.CreateInstance(__uuidof(SldWorks)))) { // No? Then start a brand new one
                throw std::runtime_error("SwApp: Could not connect to or start SolidWorks.");
            }
        }   // ^m_swApp is a Smart Pointer that uses COM for Solidwork

        if (m_swApp) {
            m_swApp->PutVisible(VARIANT_TRUE); // Pop-up Solidworks
        }
    } catch (_com_error& e) {
        // L is a flag to use 16-bit Unicode symbols, default is 8-bit
        // std::endl is very important, so in case of a crash it wont hang without output (flushes the RAM tank)
        std::wcerr << L"SwApp: COM Error during connection: " << e.ErrorMessage() << std::endl;
        throw std::runtime_error("SwApp: COM Error during connection.");
    }
}
