#include "GeometryExtractor.h"
#include "SafeArrayWrapper.h"
#include "SafeArrayBuilder.h"
#include <iostream>
#include <atlbase.h> // For CComPtr

// 'const SwPart& part' -> We pass by CONST REFERENCE.
// In C#, objects are references by default. In C++, if we forgot the '&',
// the computer would try to CLONE the entire SolidWorks model in RAM.
// 'const' promises we won't accidentally modify the original part.
ModelData GeometryExtractor::Extract(const SwPart& part) {
    ModelData result;
    IModelDoc2Ptr swModel = part.Get();
    if (!swModel) return result;

    try {
        IPartDocPtr swPart = swModel;
        if (!swPart) return result;

        // v VARIANT is a C# "dynamic", and variant_t a WRAPPER to handle it
        variant_t vBodies = swPart->GetBodies2(-1, VARIANT_FALSE);
        if (vBodies.vt & VT_ARRAY) {

            // --- BODIES ---
            SafeArrayWrapper<IBody2Ptr> bodies(vBodies.parray);
            for (long i = bodies.Lower(); i <= bodies.Upper(); i++) {
                IBody2Ptr swBody = bodies.GetAt(i);

                if (swBody) {
                    // --- FACES ---
                    variant_t vFaces = swBody->GetFaces();
                    if (vFaces.vt & VT_ARRAY) {
                        SafeArrayWrapper<IFace2Ptr> faces(vFaces.parray);
                        for (long j = faces.Lower(); j <= faces.Upper(); j++) {
                            IFace2Ptr swFace = faces.GetAt(j);
                            if (swFace) {
                                // Get Tessellation (Triangles) for the face
                                // VARIANT_TRUE means "use high-quality tessellation"
                                variant_t vTess = swFace->GetTessTriangles(VARIANT_TRUE);
                                                            // ^Break up model to list of triangles
                                if (vTess.vt == (VT_ARRAY | VT_R4)) { // Array of floats
                                    float* pFloats = nullptr;
                                    SafeArrayAccessData(vTess.parray, (void**)&pFloats);
                                    // ^Direct memory access to array (High performance)
                                    long tL, tU;
                                    SafeArrayGetLBound(vTess.parray, 1, &tL);
                                    SafeArrayGetUBound(vTess.parray, 1, &tU);
                                    // Each triangle has 3 vertices * 3 coordinates (x,y,z) = 9 floats
                                    for (long k = 0; k < (tU - tL + 1); k += 9) {
                                        result.triangles.push_back({
                                            {(double)pFloats[k], (double)pFloats[k+1], (double)pFloats[k+2]},
                                            {(double)pFloats[k+3], (double)pFloats[k+4], (double)pFloats[k+5]},
                                            {(double)pFloats[k+6], (double)pFloats[k+7], (double)pFloats[k+8]}
                                        });
                                    }
                                    SafeArrayUnaccessData(vTess.parray); // Unlock memory
                                }
                            }
                        }
                    }

                    // --- EDGES ---
                    variant_t vEdges = swBody->GetEdges();
                    if (vEdges.vt & VT_ARRAY) {
                        SafeArrayWrapper<IEdgePtr> edges(vEdges.parray);
                        for (long j = edges.Lower(); j <= edges.Upper(); j++) {
                            IEdgePtr swEdge = edges.GetAt(j);
                            if (swEdge) {
                                ICurvePtr swCurve = swEdge->GetCurve();
                                if (swCurve) {
                                    double startPt[3], endPt[3];
                                    VARIANT_BOOL closed = VARIANT_FALSE, periodic = VARIANT_FALSE;
                                    swCurve->GetEndParams(startPt, endPt, &closed, &periodic);

                                    // --- USE BUILDER FOR EDGES ---
                                    // We use our new builder to pack the 3D points into COM format.
                                    // VT_R8 means "Array of Doubles".
                                    variant_t vStart = SafeArrayBuilder::Create(startPt, 3, VT_R8);
                                    variant_t vEnd = SafeArrayBuilder::Create(endPt, 3, VT_R8);

                                    variant_t vPoly = swCurve->GetTessPts(0.001, 0.001, vStart, vEnd);
                                    if (vPoly.vt == (VT_ARRAY | VT_R8)) {
                                        SafeArrayWrapper<double> points(vPoly.parray);
                                        double* pData = nullptr;
                                        SafeArrayAccessData(vPoly.parray, (void**)&pData);
                                        
                                        std::vector<Vector3> chain;
                                        for (long k = 0; k < points.Count(); k += 3) {
                                            chain.push_back({pData[k], pData[k+1], pData[k+2]});
                                        }
                                        result.edgeChains.push_back(chain);
                                        SafeArrayUnaccessData(vPoly.parray);
                                    }
                                    // Memory cleanup is handled by variant_t RAII
                                }
                            }
                        }
                    }
                }
            }
        }
    } catch (_com_error& e) {
        std::wcerr << L"Extraction Error: " << e.ErrorMessage() << std::endl;
    }
    return result;
}
