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

SwPartPtr SolidWorksEngine::OpenPart(const std::string& filePath) {
    if (!m_app) throw std::runtime_error("Engine: No SolidWorks connection.");

    // 1. Extension Validation
    std::string ext = ".SLDPRT";
    if (filePath.length() < ext.length() ||
        filePath.compare(filePath.length() - ext.length(), ext.length(), ext) != 0)
    {
        throw std::runtime_error("Engine: File is not a .SLDPRT part.");
    }

    // 2. Define the closer (The technician who knows how to use m_app)
    auto closer = [this](SwPart* part) {
        if (part && this->m_app) {
            _bstr_t title = part->GetTitle();
            this->m_app->Get()->CloseDoc(title);
            std::cout << "Engine: State 'OpenPart' cleaned up. Document closed." << std::endl;
            delete part;
        }
    };

    // 3. Create and RETURN the state to the caller
    return SwPartPtr(new SwPart(m_app->Get(), filePath), closer);
}

void SolidWorksEngine::ConvertToObj(SwPart& part, const std::string& outputPath) {
    // --- EXTRACTION PHASE ---
    std::vector<Triangle> allTriangles;
    std::vector<std::vector<Vector3>> allEdgeChains;

    IModelDoc2Ptr swModel = part.Get();
    if (!swModel) throw std::runtime_error("Engine: Invalid model document.");

    IPartDocPtr swPart = swModel;
    if (!swPart) throw std::runtime_error("Engine: Document is not a part.");

    // v VARIANT is a C# "dynamic", and variant_t a WRAPPER to handle it
    variant_t vBodies = swPart->GetBodies2(-1, VARIANT_FALSE);
    if (!(vBodies.vt & VT_ARRAY)) throw std::runtime_error("Engine: No bodies found.");

    // --- BODIES (Using Wrapper + GetAllElements) ---
    auto bodies = SafeArrayWrapper<IBody2Ptr>(vBodies.parray).GetAllElements();

    for (auto& swBody : bodies) {
        if (!swBody) continue;

        // --- FACES ---
        variant_t vFaces = swBody->GetFaces();
        if (vFaces.vt & VT_ARRAY) {
            auto faces = SafeArrayWrapper<IFace2Ptr>(vFaces.parray).GetAllElements();

            for (auto& swFace : faces) {
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
                SafeArrayUnaccessData(vTess.parray); // Unlock memory
            }
        }

        // --- EDGES ---
        variant_t vEdges = swBody->GetEdges();
        if (vEdges.vt & VT_ARRAY) {
            auto edges = SafeArrayWrapper<IEdgePtr>(vEdges.parray).GetAllElements();

            for (auto& swEdge : edges) {
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
                if (vPoly.vt == (VT_ARRAY | VT_R8)) {
                    // We use direct memory access for points (VT_R8 = double)
                    double* pData = nullptr;
                    SafeArrayAccessData(vPoly.parray, (void**)&pData);

                    long pL, pU;
                    SafeArrayGetLBound(vPoly.parray, 1, &pL);
                    SafeArrayGetUBound(vPoly.parray, 1, &pU);
                    long count = pU - pL + 1;

                    std::vector<Vector3> chain;
                    for (long k = 0; k < count; k += 3) {
                        chain.push_back({pData[k], pData[k+1], pData[k+2]});
                    }
                    allEdgeChains.push_back(chain);
                    SafeArrayUnaccessData(vPoly.parray); // Unlock memory
                }
            }
        }
    }

    // --- EXPORT PHASE ---
    std::ofstream outFile(outputPath);
    if (!outFile.is_open()) throw std::runtime_error("Engine: Could not open output file.");

    outFile << "# Generated by SolidWorks C++ Stateless Converter" << std::endl;

    int vIdx = 1;
    for (const auto& tri : allTriangles) {
        outFile << "v " << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << "\n"
                << "v " << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << "\n"
                << "v " << tri.v3.x << " " << tri.v3.y << " " << tri.v3.z << "\n"
                << "f " << vIdx << " " << vIdx + 1 << " " << vIdx + 2 << "\n";
        vIdx += 3;
    }

    for (const auto& chain : allEdgeChains) {
        int sIdx = vIdx;
        for (const auto& pt : chain) {
            outFile << "v " << pt.x << " " << pt.y << " " << pt.z << "\n";
            vIdx++;
        }
        outFile << "l";
        for (int i = 0; i < (int)chain.size(); ++i) outFile << " " << (sIdx + i);
        outFile << "\n";
    }

    std::cout << "Engine: Export finished (" << allTriangles.size() << " triangles)." << std::endl;
}
