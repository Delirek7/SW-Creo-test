#pragma once
#include "SwApp.h"
#include <string>
#include <memory>

class SwPart {
public:
    // Factory Method: Returns a smart pointer to an open part, or nullptr if it fails.
    static std::unique_ptr<SwPart> Open(SwApp& app, const std::string& filePath);

    // Destructor: AUTOMATICALLY closes the part in SolidWorks
    ~SwPart();

    // Returns the raw Model pointer
    IModelDoc2Ptr Get() const { return m_swModel; }

private:
    // We keep a reference to the app so we can tell it to close the doc later
    // Constructor is PRIVATE
    SwPart(SwApp& app, IModelDoc2Ptr swModel);

    SwApp& m_app;
    IModelDoc2Ptr m_swModel;
};
