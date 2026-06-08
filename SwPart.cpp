#include "SwPart.h"
#include <iostream>

SwPart::SwPart(ISldWorksPtr app, const std::string& filePath) : m_app(app) {
    if (!m_app) {
        throw std::runtime_error("SwPart: SolidWorks application pointer is null.");
    }

    long errors = 0;
    long warnings = 0;

                // v dot (.) is used for Objects (s.length), and the arrow is used for Pointers
    m_swModel = m_app->OpenDoc6(    // OpenDoc6 takes 6 arguments
        _bstr_t(filePath.c_str()),  // 1.   PATH
        // ^_bstr_t is a "bridge" (WRAPPER class) between std::string and BSTR. Solidworks is expecting BSTR Path and Configuration
        1,                          // 2.   TYPE: 1 = Part (.sldprt), 2 = Assembly (.sldasm), 3 = Drawing (.slddrw) 
        1,                          // 3.   OPTIONS: 1 = Silent, 2 = Read only, 4 = View only, 8 = Rapid Draw, 16 = Load only
        _bstr_t(""),                // 4.   CONFIG: empty string = default
        &errors, &warnings          // 5, 6 OUTPUT: Store errors/warnings here
    );

    // If m_swModel is null, the "Promise" of the constructor is broken.
    if (!m_swModel) {
        throw std::runtime_error("SwPart: Failed to open file: " + filePath);
    }
}

SwPart::~SwPart() {
    if (m_swModel && m_app) {
        try {
            _bstr_t title = m_swModel->GetTitle();
            m_app->CloseDoc(title);
            // Destructor handles cleanup automatically when the variable dies (RAII)
            std::cout << "SwPart: Document closed automatically (RAII)." << std::endl;
        } catch (...) {
            // Destructors MUST NOT throw
        }
    }
}
