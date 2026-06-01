#pragma once
#include "SwApp.h"
#include <string>

class SwPart {
public:
    // Constructor: Opens the specified part file
    SwPart(SwApp& app, const std::string& filePath);

    // Destructor: AUTOMATICALLY closes the part in SolidWorks
    ~SwPart();

    // Returns the raw Model pointer
    IModelDoc2Ptr Get() const { return m_swModel; }

    // Check if the file was opened successfully
    bool IsValid() const { return m_swModel != nullptr; }

private:
    // We keep a reference to the app so we can tell it to close the doc later
    SwApp& m_app;
    IModelDoc2Ptr m_swModel;
};
