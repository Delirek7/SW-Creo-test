#include "GeometryExtractor.h"
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

        // v VARIANT is a C# "dynamic", and variant_t a wrapper to handle it
        variant_t vBodies = swPart->GetBodies2(-1, VARIANT_FALSE);
        if (vBodies.vt & VT_ARRAY) { // Ensure that vBodies is an array with bitwise AND (&)
            SAFEARRAY* psa = vBodies.parray;
            long lowerBound, upperBound;
            SafeArrayGetLBound(psa, 1, &lowerBound);
            SafeArrayGetUBound(psa, 1, &upperBound);

            for (long i = lowerBound; i <= upperBound; i++) {
                CComPtr<IUnknown> pUnk;
                SafeArrayGetElement(psa, &i, &pUnk.p); // <- SafeArrayGetElement requests IUnknown pointer (pUnk)
                IBody2Ptr swBody = pUnk.p;

                if (swBody) {
                    // --- FACES ---
                    variant_t vFaces = swBody->GetFaces();
                    if (vFaces.vt & VT_ARRAY) {
                        SAFEARRAY* psaFaces = vFaces.parray;
                        long fL, fU;
                        SafeArrayGetLBound(psaFaces, 1, &fL);
                        SafeArrayGetUBound(psaFaces, 1, &fU);

                        for (long j = fL; j <= fU; j++) {
                            CComPtr<IUnknown> pUnkFace;
                            SafeArrayGetElement(psaFaces, &j, &pUnkFace.p);
                            IFace2Ptr swFace = pUnkFace.p;
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
                        SAFEARRAY* psaEdges = vEdges.parray;
                        long eL, eU;
                        SafeArrayGetLBound(psaEdges, 1, &eL);
                        SafeArrayGetUBound(psaEdges, 1, &eU);

                        for (long j = eL; j <= eU; j++) {
                            CComPtr<IUnknown> pUnkEdge;
                            SafeArrayGetElement(psaEdges, &j, &pUnkEdge.p);
                            IEdgePtr swEdge = pUnkEdge.p;
                            if (swEdge) {
                                ICurvePtr swCurve = swEdge->GetCurve();
                                if (swCurve) {
                                    double startPt[3], endPt[3];
                                    VARIANT_BOOL closed = VARIANT_FALSE, periodic = VARIANT_FALSE;
                                    swCurve->GetEndParams(startPt, endPt, &closed, &periodic);

                                    // Packing points into SAFEARRAYs for high-speed direct access
                                    SAFEARRAYBOUND bounds = { 3, 0 };
                                    SAFEARRAY *psaStart = SafeArrayCreate(VT_R8, 1, &bounds);
                                    SAFEARRAY *psaEnd = SafeArrayCreate(VT_R8, 1, &bounds);
                                    double *pD1, *pD2;
                                    SafeArrayAccessData(psaStart, (void**)&pD1);
                                    SafeArrayAccessData(psaEnd, (void**)&pD2);
                                    memcpy(pD1, startPt, 3 * sizeof(double));
                                    memcpy(pD2, endPt, 3 * sizeof(double));
                                    SafeArrayUnaccessData(psaStart);
                                    SafeArrayUnaccessData(psaEnd);

                                    variant_t vStart; vStart.vt = VT_ARRAY | VT_R8; vStart.parray = psaStart;
                                    variant_t vEnd; vEnd.vt = VT_ARRAY | VT_R8; vEnd.parray = psaEnd;

                                    variant_t vPoly = swCurve->GetTessPts(0.001, 0.001, vStart, vEnd);
                                    if (vPoly.vt == (VT_ARRAY | VT_R8)) {
                                        double* pData = nullptr;
                                        SafeArrayAccessData(vPoly.parray, (void**)&pData);
                                        long pL, pU;
                                        SafeArrayGetLBound(vPoly.parray, 1, &pL);
                                        SafeArrayGetUBound(vPoly.parray, 1, &pU);
                                        
                                        std::vector<Vector3> chain;
                                        for (long k = 0; k < (pU - pL + 1); k += 3) {
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
