#include "SwApp.h"

SwApp::SwApp() {
    // Constructing connection
    try {                                                           // Is SolidWorks already open?
        if (FAILED(m_swApp.GetActiveObject(__uuidof(SldWorks)))) {  // __uuidof search for app ID in Win Registry
            if (FAILED(m_swApp.CreateInstance(__uuidof(SldWorks)))) { // No? Then start a brand new one
                throw std::runtime_error("SwApp: Could not connect to or start SolidWorks.");
            }
        }

        if (m_swApp) {
            m_swApp->PutVisible(VARIANT_TRUE); // Pop-up Solidworks
        }
    } catch (_com_error&) {
        throw std::runtime_error("SwApp: COM Error during connection.");
    }
}
