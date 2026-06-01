#pragma once
#include "SwPart.h"
#include "IConverterEngine.h" // For Vector3, Triangle, LineSegment structs
#include <vector>

// Bundles all extracted data into one package
struct ModelData {
    std::vector<Triangle> triangles;
    std::vector<std::vector<Vector3>> edgeChains;
};

class GeometryExtractor {
public:
    // Static method: This is a "Stateless Service" (like a Static Class in C#).
    // It doesn't store data; it just performs a transformation. This makes
    // the code easier to test and saves memory.
    static ModelData Extract(const SwPart& part);
};
