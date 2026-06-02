#pragma once
#include <windows.h>
#include <comdef.h>
#include <memory>

// Import SolidWorks
#import "C:\\Program Files\\SOLIDWORKS Corp\\SOLIDWORKS\\sldworks.tlb" no_namespace, named_guids

class SwApp {
public:
    // Factory Method: Returns a smart pointer to a connected app, or nullptr if it fails.
    static std::unique_ptr<SwApp> Connect();

    ~SwApp() = default;

    ISldWorksPtr Get() const { return m_swApp; }

private:
    // Constructor is PRIVATE so users MUST use the Connect() factory.
    SwApp(ISldWorksPtr swApp);
    ISldWorksPtr m_swApp;
};
