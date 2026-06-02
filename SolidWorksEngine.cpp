#include "SolidWorksEngine.h"
#include "GeometryExtractor.h"
#include "ObjExporter.h"
#include <iostream>

SolidWorksEngine::SolidWorksEngine() {
    // Try to connect on startup
    m_app = SwApp::Connect();
}

bool SolidWorksEngine::OpenPart(const std::string& filePath) {
    if (!m_app) {
        std::cerr << "Engine Error: No SolidWorks connection." << std::endl;
        return false;
    }

    // Try to open the part
    m_activePart = SwPart::Open(*m_app, filePath);

    if (!m_activePart) {
        std::cerr << "Engine Error: Could not open file: " << filePath << std::endl;
        return false;
    }

    return true;
}

bool SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    if (!m_activePart) return false;

    // Orchestration with no try-catch
    ModelData data = GeometryExtractor::Extract(*m_activePart);
    return ObjExporter::Save(data, outputPath);
}

void SolidWorksEngine::ClosePart() {
    m_activePart.reset();
}
