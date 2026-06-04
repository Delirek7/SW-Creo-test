#pragma once
#include <memory>
#include <string>
#include "IConverterEngine.h"
#include "SwApp.h"
#include "SwPart.h"

class SolidWorksEngine : public IConverterEngine {
public:
    SolidWorksEngine();
    virtual ~SolidWorksEngine() = default;

    // Overriding interface with void return types
    void OpenPart(const std::string& filePath) override;
    void ConvertToObj(const std::string& outputPath) override;
    void ClosePart() override;

private:
    std::unique_ptr<SwApp> m_app;
    std::unique_ptr<SwPart> m_activePart;
};
