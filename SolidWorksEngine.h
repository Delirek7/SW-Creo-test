#pragma once
#include <memory>
#include <string>
#include <functional> // For std::function
#include "IConverterEngine.h"
#include "SwApp.h"
#include "SwPart.h"

class SolidWorksEngine : public IConverterEngine {
public:
    SolidWorksEngine();
    virtual ~SolidWorksEngine() = default;

    void OpenPart(const std::string& filePath) override;
    void ConvertToObj(const std::string& outputPath) override;
    void ClosePart() override;

private:
    std::unique_ptr<SwApp> m_app;

    // We use a custom deleter. This is a "Smart Function" that runs when the part dies.
    // It allows us to close the document using m_app WITHOUT SwPart knowing about m_app.
    using SwPartPtr = std::unique_ptr<SwPart, std::function<void(SwPart*)>>;
    SwPartPtr m_activePart;
};
