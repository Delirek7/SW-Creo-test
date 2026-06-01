#pragma once
#include "GeometryExtractor.h"
#include <string>

class ObjExporter {
public:
    // Takes extracted model data and saves it to an OBJ file
    static bool Save(const ModelData& data, const std::string& outputPath);
};
