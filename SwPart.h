#pragma once
#include "SwApp.h"
#include <string>
#include <stdexcept>

class SwPart {
public:
    // Constructor: GUARANTEES the file is open or throws an exception.
    SwPart(SwApp& app, const std::string& filePath);

    ~SwPart();

    // Guaranteed valid document pointer
    IModelDoc2Ptr Get() const { return m_swModel; }

private:
    SwApp& m_app;
    IModelDoc2Ptr m_swModel;
};
