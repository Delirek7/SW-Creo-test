#pragma once
#include <windows.h>
#include <comdef.h>
#include <string>
#include <stdexcept>

// Import SolidWorks for COM types
#import "C:\\Program Files\\SOLIDWORKS Corp\\SOLIDWORKS\\sldworks.tlb" no_namespace, named_guids

class SwPart {
public:
    // Constructor: Takes the app pointer and path to open the file.
    // It uses the app but does NOT store it, keeping this class pure.
    SwPart(ISldWorksPtr app, const std::string& filePath);

    // Destructor: Empty because the Orchestrator handles the closing logic.
    ~SwPart() = default;

    // Returns the raw Model pointer
    IModelDoc2Ptr Get() const { return m_swModel; }

    // Helper to get the title (needed by the Orchestrator's closer)
    _bstr_t GetTitle() const { return m_swModel->GetTitle(); }

private:
    IModelDoc2Ptr m_swModel;
};
