#include "SwApp.h"

SwApp::SwApp() {
    // Constructing connection
    try {                                                           // Is SolidWorks already open?
        if (FAILED(m_swApp.GetActiveObject(__uuidof(SldWorks)))) {  // __uuidof search for app ID in Win Registry
            m_swApp.CreateInstance(__uuidof(SldWorks));             // No? Then start a brand new one
        }   // ^m_swApp is a Smart Pointer that uses COM for Solidwork

        if (m_swApp) {
            m_swApp->PutVisible(VARIANT_TRUE); // Pop-up Solidworks
        }
    } catch (_com_error& e) {
        std::wcerr << L"SwApp Connection Error: " << e.ErrorMessage() << std::endl;
    }               //^L is a flag to use 16-bit Unicode symbols, default is 8-bit
}
