#pragma once

#include <string>

// This is our "Contract" (Interface).
// It defines WHAT the converter does, but not HOW it does it.
class IConverterEngine {
public:
    // Virtual destructor is REQUIRED for C++ interfaces to prevent memory leaks
    virtual ~IConverterEngine() = default;

    // Pure virtual function (= 0 means "must be implemented by child class")
    // std::string is used here, and we pass by "const reference" (&) for performance.
    virtual bool OpenPart(const std::string& filePath) = 0;

    // Extracts faces and edges and saves them to the output path
    virtual bool ConvertToObj(const std::string& outputPath) = 0;

    // Optional: We might want a way to clean up or close the document explicitly
    virtual void ClosePart() = 0;
};

// A simple structure to hold 3D point data
struct Vector3 {
    double x, y, z;
};

// A triangle is made of 3 points
struct Triangle {
    Vector3 v1, v2, v3;
};

// A simple line segment between two points
struct LineSegment {
    Vector3 v1, v2;
};
