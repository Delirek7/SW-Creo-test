#pragma once
#include <memory>
#include <string>
#include <vector>
#include "IConverterEngine.h"
#include "SwApp.h"
#include "SwPart.h"
#include "GeometryExtractor.h"
#include "ObjExporter.h"

// The Orchestrator class
class SolidWorksEngine : public IConverterEngine {
public:
    SolidWorksEngine() = default;
    virtual ~SolidWorksEngine() = default;

    bool OpenPart(const std::string& filePath) override;
    bool ConvertToObj(const std::string& outputPath) override;
    void ClosePart() override;

private:
    SwApp m_app;
    std::unique_ptr<SwPart> m_activePart;
    std::string m_currentPath; // To keep track of the opened path
};
