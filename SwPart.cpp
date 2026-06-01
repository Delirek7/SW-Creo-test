#include "SwPart.h"
#include <iostream>

SwPart::SwPart(SwApp& app, const std::string& filePath) : m_app(app) {
    if (!app.IsValid()) return;

    try {
        long errors = 0;
        long warnings = 0;
                        //  v dot (.) is used for Objects (s.length), and the arrow is used for Pointers
        m_swModel = app.Get()->OpenDoc6(  // OpenDoc6 takes 6 arguments
            _bstr_t(filePath.c_str()),  // 1.   PATH
            // ^_bstr_t is a "bridge" (wrapper class) between std::string and BSTR. Solidworks is expecting BSTR Path and Configuration
            1,                          // 2.   TYPE: 1 = Part (.sldprt), 2 = Assembly (.sldasm), 3 = Drawing (.slddrw) 
            1,                          // 3.   OPTIONS: 1 = Silent, 2 = Read only, 4 = View only, 8 = Rapid Draw, 16 = Load only
            _bstr_t(""),                // 4.   CONFIG: empty string = default
            &errors, &warnings          // 5, 6 OUTPUT: Store errors/warnings here
        );
    } catch (_com_error& e) {
        std::wcerr << L"SwPart Open Error: " << e.ErrorMessage() << std::endl;
    }
}

SwPart::~SwPart() {
    try {
        if (m_app.IsValid() && m_swModel) {
            _bstr_t title = m_swModel->GetTitle();
            m_app.Get()->CloseDoc(title);
            // Destructor handles cleanup automatically when the variable dies (RAII)
            std::cout << "SwPart: Document '" << (const char*)title << "' closed automatically." << std::endl;
        }
    } catch (_com_error&) {
        // Destructors should never throw exceptions
    }
}
