#include "SwApp.h"

SwApp::SwApp(ISldWorksPtr swApp) : m_swApp(swApp) {}

std::unique_ptr<SwApp> SwApp::Connect() {
    ISldWorksPtr swApp;

    // Try to connect
    if (FAILED(swApp.GetActiveObject(__uuidof(SldWorks)))) {
        if (FAILED(swApp.CreateInstance(__uuidof(SldWorks)))) {
            return nullptr; // Failure: Return null instead of throwing
        }
    }

    if (swApp) {
        swApp->PutVisible(VARIANT_TRUE);
        // Create the object and wrap it in a unique_ptr
        // We use 'new' here because the constructor is private
        return std::unique_ptr<SwApp>(new SwApp(swApp));
    }

    return nullptr;
}
