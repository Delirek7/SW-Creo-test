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

    bool OpenPart(const std::string& filePath) override;
    bool ConvertToObj(const std::string& outputPath) override;
    void ClosePart() override;

private:
    // m_app is now a pointer because it might fail to connect
    std::unique_ptr<SwApp> m_app;
    std::unique_ptr<SwPart> m_activePart;
};
