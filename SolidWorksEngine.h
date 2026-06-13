#pragma once
#include "IConverterEngine.h"
#include "SwApp.h"
#include "SwPart.h"

class SolidWorksEngine : public IConverterEngine {
public:
    SolidWorksEngine();
    virtual ~SolidWorksEngine() = default;

    // Now returns the part (The State) instead of storing it
    SwPartPtr OpenPart(const std::string& filePath) override;

    // Now accepts the part as a parameter
    void ConvertToObj(SwPart& part, const std::string& outputPath) override;

private:
    std::unique_ptr<SwApp> m_app;
};
