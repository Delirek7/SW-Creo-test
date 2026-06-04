#include "SolidWorksEngine.h"
#include "GeometryExtractor.h"
#include "ObjExporter.h"

SolidWorksEngine::SolidWorksEngine() {
    // Exception propagates up to the caller
    m_app = std::make_unique<SwApp>();
}

void SolidWorksEngine::OpenPart(const std::string& filePath) {
    // 1. Extension Validation
    // We check if the file is actually a SolidWorks Part before telling SW to open it.
    std::string ext = ".SLDPRT";
    if (filePath.length() < ext.length() ||
        filePath.compare(filePath.length() - ext.length(), ext.length(), ext) != 0)
    {
        // We convert to uppercase for the check to be case-insensitive if needed, 
        // but for now, we'll do a simple comparison.
        throw std::runtime_error("Engine: File is not a .SLDPRT part.");
    }

    // If this fails, SwPart throws, and execution jumps straight to main's catch block.
    // No more manual cleanup needed here!
    m_activePart = std::make_unique<SwPart>(*m_app, filePath);
}

void SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    if (!m_activePart) {
        throw std::runtime_error("Engine: Cannot convert because no part is open.");
    }

    // Extraction and Export errors will propagate automatically
    ModelData data = GeometryExtractor::Extract(*m_activePart);
    ObjExporter::Save(data, outputPath);
}

void SolidWorksEngine::ClosePart() {
    m_activePart.reset();
}
