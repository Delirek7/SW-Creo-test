#include "SolidWorksEngine.h"
#include <iostream>

SolidWorksEngine::SolidWorksEngine() {
    try {                                                           // Is SolidWorks already open?
        if (FAILED(m_swApp.GetActiveObject(__uuidof(SldWorks)))) {
            m_swApp.CreateInstance(__uuidof(SldWorks));             // No? Then start a brand new one
        }

        if (m_swApp) {
            // Use PutVisible instead of .Visible property
            m_swApp->PutVisible(VARIANT_TRUE);
        }
    } catch (_com_error& e) {
        std::wcerr << L"SolidWorks Init Error: " << e.ErrorMessage() << std::endl;
    }
}

bool SolidWorksEngine::OpenPart(const std::string& filePath) {
    if (!m_swApp) return false;

    try {
        long errors = 0;
        long warnings = 0;

        m_swModel = m_swApp->OpenDoc6(  // OpenDoc6 takes 6 arguments
            _bstr_t(filePath.c_str()),  // PATH
            1,                          // TYPE: 1 = Part (.sldprt)
            1,                          // OPTIONS: 1 = Silent
            _bstr_t(""),                // CONFIG: empty string = default
            &errors, &warnings          // OUTPUT: Store errors/warnings here
        );

        return m_swModel != nullptr;    // RETURN true if we actually got a document
    } catch (_com_error&) {
        return false;
    }
}

void SolidWorksEngine::ClosePart() {
    try {
        if (m_swApp && m_swModel) {
            // GetTitle can sometimes return HRESULT even in wrapper mode
            // We'll use the most explicit way to get the string
            _bstr_t title = m_swModel->GetTitle();
            
            // CloseDoc might require the second argument (VARIANT_BOOL*) in some wrappers
            // We pass a dummy variable for it.
            VARIANT_BOOL closed = VARIANT_FALSE;
            m_swApp->CloseDoc(title); 
            
            m_swModel = nullptr;
        }
    } catch (_com_error&) {
    }
}

bool SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    // This will be our "Big Task" - extracting faces/edges.
    // For now, let's just return true to verify our structure works.
    return true;
}
