#include "SwPart.h"
#include <iostream>

SwPart::SwPart(SwApp& app, IModelDoc2Ptr swModel) : m_app(app), m_swModel(swModel) {}

std::unique_ptr<SwPart> SwPart::Open(SwApp& app, const std::string& filePath) {
    long errors = 0;
    long warnings = 0;

                            // v dot (.) is used for Objects (s.length), and the arrow is used for Pointers
    IModelDoc2Ptr swModel = app.Get()->OpenDoc6(    // OpenDoc6 takes 6 arguments
        _bstr_t(filePath.c_str()),  // 1.   PATH
        // ^_bstr_t is a "bridge" (wrapper class) between std::string and BSTR. Solidworks is expecting BSTR Path and Configuration
        1,                          // 2.   TYPE: 1 = Part (.sldprt), 2 = Assembly (.sldasm), 3 = Drawing (.slddrw) 
        1,                          // 3.   OPTIONS: 1 = Silent, 2 = Read only, 4 = View only, 8 = Rapid Draw, 16 = Load only
        _bstr_t(""),                // 4. CONFIG: empty string = default
        &errors, &warnings          // 5, 6 OUTPUT: Store errors/warnings here
    );

    if (swModel) {
        return std::unique_ptr<SwPart>(new SwPart(app, swModel));
    }

    return nullptr; // Failure: Return null instead of throwing
}

SwPart::~SwPart() {
    if (m_swModel) {
        _bstr_t title = m_swModel->GetTitle();
        m_app.Get()->CloseDoc(title);
        std::cout << "SwPart: Document closed automatically." << std::endl;
    }
}
