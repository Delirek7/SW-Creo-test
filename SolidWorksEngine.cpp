#include "SolidWorksEngine.h"
#include "SafeArrayWrapper.h"
#include "SafeArrayBuilder.h"
#include <iostream>
#include <fstream>
#include <vector>
#include <atlbase.h> // For CComPtr

SolidWorksEngine::SolidWorksEngine() {
    // Connection happens inside SwApp constructor (RAII)
    m_app = std::make_unique<SwApp>();
}

void SolidWorksEngine::OpenPart(const std::string& filePath) {
    if (!m_app) throw std::runtime_error("Engine: No SolidWorks connection.");

    // 1. Extension Validation (Before we tell SolidWorks to do anything heavy)
    std::string ext = ".SLDPRT";
    if (filePath.length() < ext.length() ||
        filePath.compare(filePath.length() - ext.length(), ext.length(), ext) != 0)
    {
        throw std::runtime_error("Engine: File is not a .SLDPRT part.");
    }

    // 2. The Orchestrator simply delegates the opening to the SwPart specialist.
    // The SwPart constructor handles the actual OpenDoc6 call.
    // If it fails, it throws, and main's catch block will handle it.
    m_activePart = SwPartPtr(new SwPart(m_app->Get(), filePath), [this](SwPart* part) {
        if (part) {
            _bstr_t title = part->GetTitle();
            this->m_app->Get()->CloseDoc(title);
            std::cout << "Engine: Document '" << (const char*)title << "' closed automatically by Orchestrator." << std::endl;
            delete part;
        }
    });
}

void SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    if (!m_activePart) throw std::runtime_error("Engine: Cannot convert because no part is open.");

    // --- EXTRACTION PHASE ---
    std::vector<Triangle> allTriangles;
    std::vector<std::vector<Vector3>> allEdgeChains;

    IModelDoc2Ptr swModel = m_activePart->Get();
    if (!swModel) throw std::runtime_error("Engine: Invalid model document.");

    IPartDocPtr swPart = swModel;
    if (!swPart) throw std::runtime_error("Engine: Document is not a part.");

    // v VARIANT is a C# "dynamic", and variant_t a WRAPPER to handle it
    variant_t vBodies = swPart->GetBodies2(-1, VARIANT_FALSE);
    if (!(vBodies.vt & VT_ARRAY)) { // Ensure that vBodies is an array with bitwise AND (&)
        throw std::runtime_error("Engine: No bodies found or invalid array returned.");
    }

    // --- BODIES (Using Wrapper) ---
    SafeArrayWrapper<IBody2Ptr> bodies(vBodies.parray);

    for (long i = bodies.Lower(); i <= bodies.Upper(); i++) {
        IBody2Ptr swBody = bodies.GetAt(i);
        if (!swBody) continue; // Early exit (Continue) to flatten nesting levels

        // --- FACES ---
        variant_t vFaces = swBody->GetFaces();
        if (vFaces.vt & VT_ARRAY) {
            SafeArrayWrapper<IFace2Ptr> faces(vFaces.parray);
            for (long j = faces.Lower(); j <= faces.Upper(); j++) {
                IFace2Ptr swFace = faces.GetAt(j);
                if (!swFace) continue;

                // VARIANT_TRUE means "use high-quality tessellation"
                variant_t vTess = swFace->GetTessTriangles(VARIANT_TRUE);
                                            // ^Break up model to list of triangles
                if (!(vTess.vt & VT_ARRAY)) continue;

                float* pFloats = nullptr;
                SafeArrayAccessData(vTess.parray, (void**)&pFloats);
                // ^Direct memory access to array (High performance - like Shared Memory)
                long tL, tU;
                SafeArrayGetLBound(vTess.parray, 1, &tL);
                SafeArrayGetUBound(vTess.parray, 1, &tU);

                // Each triangle has 3 vertices * 3 coordinates (x,y,z) = 9 floats
                for (long k = 0; k < (tU - tL + 1); k += 9) {
                    allTriangles.push_back({
                        {(double)pFloats[k], (double)pFloats[k+1], (double)pFloats[k+2]},
                        {(double)pFloats[k+3], (double)pFloats[k+4], (double)pFloats[k+5]},
                        {(double)pFloats[k+6], (double)pFloats[k+7], (double)pFloats[k+8]}
                    });
                }
                SafeArrayUnaccessData(vTess.parray); // Unlock memory (Unlock the Suitcase)
            }
        }

        // --- EDGES ---
        variant_t vEdges = swBody->GetEdges();
        if (vEdges.vt & VT_ARRAY) {
            SafeArrayWrapper<IEdgePtr> edges(vEdges.parray);
            for (long j = edges.Lower(); j <= edges.Upper(); j++) {
                IEdgePtr swEdge = edges.GetAt(j);
                if (!swEdge) continue;

                ICurvePtr swCurve = swEdge->GetCurve();
                if (!swCurve) continue;

                double startPt[3], endPt[3];
                VARIANT_BOOL closed = VARIANT_FALSE, periodic = VARIANT_FALSE;
                swCurve->GetEndParams(startPt, endPt, &closed, &periodic);

                // --- USE BUILDER FOR EDGES ---
                variant_t vStart = SafeArrayBuilder::Create(startPt, 3, VT_R8);
                variant_t vEnd = SafeArrayBuilder::Create(endPt, 3, VT_R8);

                variant_t vPoly = swCurve->GetTessPts(0.001, 0.001, vStart, vEnd);
                if (!(vPoly.vt & VT_ARRAY)) continue;

                SafeArrayWrapper<double> points(vPoly.parray);
                double* pData = nullptr;
                SafeArrayAccessData(vPoly.parray, (void**)&pData);

                std::vector<Vector3> chain;
                for (long k = 0; k < points.Count(); k += 3) {
                    chain.push_back({pData[k], pData[k+1], pData[k+2]});
                }
                allEdgeChains.push_back(chain);
                SafeArrayUnaccessData(vPoly.parray); // Unlock the suitcase
            }
        }
    }

    // --- EXPORT PHASE ---
    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) {
        throw std::runtime_error("Engine: Could not open file for writing: " + outputPath);
    }

    // Standard OBJ file format headers
    outFile << "# Generated by SolidWorks C++ Converter" << std::endl;

    int vertexIndex = 1; // OBJ is 1-based index

    // 1. Write Faces (Triangles)
    for (const auto& tri : allTriangles) {
        // v dot (.) is used for Objects (tri.v1.x)
        outFile << "v " << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << "\n";
        outFile << "v " << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << "\n";
        outFile << "v " << tri.v3.x << " " << tri.v3.y << " " << tri.v3.z << "\n";

        outFile << "f " << vertexIndex << " " << vertexIndex + 1 << " " << vertexIndex + 2 << "\n";
        vertexIndex += 3;
    }

    // 2. Write Edges (Continuous Chains)
    for (const auto& chain : allEdgeChains) {
        int startIdx = vertexIndex;
        for (const auto& pt : chain) {
            outFile << "v " << pt.x << " " << pt.y << " " << pt.z << "\n";
            vertexIndex++;
        }
        outFile << "l"; // Connect the dots
        for (int i = 0; i < chain.size(); ++i) {
            outFile << " " << (startIdx + i);
        }
        outFile << "\n";
    }

    outFile.close();
    std::cout << "Engine: Successfully exported " << allTriangles.size() << " triangles and "
              << allEdgeChains.size() << " edge chains to " << outputPath << std::endl;
}

void SolidWorksEngine::ClosePart() {
    m_activePart.reset();
}
