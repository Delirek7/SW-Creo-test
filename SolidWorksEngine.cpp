#include "SolidWorksEngine.h"
#include <memory>

bool SolidWorksEngine::OpenPart(const std::string& filePath) {
    m_currentPath = filePath;
    m_activePart = std::make_unique<SwPart>(m_app, filePath);
    return m_activePart->IsValid();
}

bool SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    if (!m_activePart || !m_activePart->IsValid()) return false;

    // Orchestration logic
    ModelData data = GeometryExtractor::Extract(*m_activePart);
    return ObjExporter::Save(data, outputPath);
}

void SolidWorksEngine::ClosePart() {
    m_activePart.reset();
}
