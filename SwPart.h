#pragma once
#include <windows.h>
#include <comdef.h>
#include <string>
#include <stdexcept>

// Import SolidWorks for COM pointers
#import "C:\\Program Files\\SOLIDWORKS Corp\\SOLIDWORKS\\sldworks.tlb" no_namespace, named_guids

class SwPart {
public:
    // Constructor: GUARANTEES the file is open or throws an exception.
    SwPart(ISldWorksPtr app, const std::string& filePath);

    ~SwPart();

    // Guaranteed valid document pointer
    IModelDoc2Ptr Get() const { return m_swModel; }

private:
    ISldWorksPtr m_app;
    IModelDoc2Ptr m_swModel;
};
