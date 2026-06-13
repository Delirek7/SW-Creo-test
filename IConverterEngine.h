#pragma once
#include <string>
#include <memory>
#include <functional>
#include "SwPart.h"

// Forward declaration of SwPart so we don't need the full header here
class SwPart;

// Define our Smart Pointer type with the custom closer
using SwPartPtr = std::unique_ptr<SwPart, std::function<void(SwPart*)>>;

// This is our "Stateless" Contract.
class IConverterEngine {
public:
    virtual ~IConverterEngine() = default;

    // Returns the "State" (the open part)
    virtual SwPartPtr OpenPart(const std::string& filePath) = 0;

    // Operates on a provided "State"
    virtual void ConvertToObj(SwPart& part, const std::string& outputPath) = 0;
};

// Structures for 3D data (Triangles/Edges) remain the same
struct Vector3 { double x, y, z; };
struct Triangle { Vector3 v1, v2, v3; };
