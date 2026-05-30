#include "SolidWorksEngine.h"
#include <iostream>
#include <vector>
#include <fstream>

SolidWorksEngine::SolidWorksEngine() {
        // Constructing connection
    try {                                                           // Is SolidWorks already open?
        if (FAILED(m_swApp.GetActiveObject(__uuidof(SldWorks)))) {  // __uuidof search for app ID in Win Registry
            m_swApp.CreateInstance(__uuidof(SldWorks));             // No? Then start a brand new one
        }   // ^m_swApp is a Smart Pointer that uses COM for Solidwork

        if (m_swApp) {
            m_swApp->PutVisible(VARIANT_TRUE); // Pop-up Solidworks
        }
    } catch (_com_error& e) {
        std::wcerr << L"SolidWorks Init Error: " << e.ErrorMessage() << std::endl;
    }               //^L is a flag to use 16-bit Unicode symbols, default is 8-bit
}                                                                       // ^std::endl is a veri important, so in case
                                                                        // of a plugin crash it wont hang without output
bool SolidWorksEngine::OpenPart(const std::string& filePath) {
    if (!m_swApp) return false;

    try {
        long errors = 0;
        long warnings = 0;
                        //  v dot (.) is used for Objects (s.length), and the arrow is used for Pointers
        m_swModel = m_swApp->OpenDoc6(  // OpenDoc6 takes 6 arguments
            _bstr_t(filePath.c_str()),  // 1.   PATH
            // ^_bstr_t is a "bridge" (wrapper class) between std::string and BSTR. Solidworks is expecting BSTR Path and Configuration
            1,                          // 2.   TYPE: 1 = Part (.sldprt), 2 = Assembly (.sldasm), 3 = Drawing (.slddrw) 
            1,                          // 3.   OPTIONS: 1 = Silent, 2 = Read only, 4 = View only, 8 = Rapid Draw, 16 = Load only
            _bstr_t(""),                // 4.   CONFIG: empty string = default
            &errors, &warnings          // 5, 6 OUTPUT: Store errors/warnings here
        );

        return m_swModel != nullptr;    // RETURN true if we actually got a document
    } catch (_com_error&) {
        return false;
    }
}

void SolidWorksEngine::ClosePart() {
    try {
        if (m_swApp && m_swModel) {
            // GetTitle can sometimes return HRESULT even in wrapper mode
            // We'll use the most explicit way to get the string
            _bstr_t title = m_swModel->GetTitle();
            
            // CloseDoc might require the second argument (VARIANT_BOOL*) in some wrappers
            // We pass a dummy variable for it.
            VARIANT_BOOL closed = VARIANT_FALSE;
            m_swApp->CloseDoc(title); 
            
            m_swModel = nullptr;
        }
    } catch (_com_error&) {
    }
}

bool SolidWorksEngine::ConvertToObj(const std::string& outputPath) {
    // Converting is a triple-nested loop: Bodies -> Faces -> Triangles.
    if (!m_swModel) return false;

    try {
        IPartDocPtr swPart = m_swModel;
        if (!swPart) return false;

        std::vector<Triangle> allTriangles;
        // v VARIANT is a C# "dynamic", and variant_t a wrapper to handle it
        variant_t vBodies = swPart->GetBodies2(-1, VARIANT_FALSE);
        if (vBodies.vt & VT_ARRAY) { // Ensure that vBodies is an array with bitwise AND (&)
            SAFEARRAY* psa = vBodies.parray;
            long lowerBound, upperBound;
            SafeArrayGetLBound(psa, 1, &lowerBound);
            SafeArrayGetUBound(psa, 1, &upperBound);

            for (long i = lowerBound; i <= upperBound; i++) {
                IUnknown* pUnk = nullptr;
                SafeArrayGetElement(psa, &i, &pUnk); // <- SafeArrayGetElement requests IUnknown pointer (pUnk)
                IBody2Ptr swBody = pUnk;
                if (swBody) {
                    variant_t vFaces = swBody->GetFaces();
                    if (vFaces.vt & VT_ARRAY) {
                        SAFEARRAY* psaFaces = vFaces.parray;
                        long fLBound, fUBound;
                        SafeArrayGetLBound(psaFaces, 1, &fLBound);
                        SafeArrayGetUBound(psaFaces, 1, &fUBound);

                        for (long j = fLBound; j <= fUBound; j++) {
                            IUnknown* pUnkFace = nullptr;
                            SafeArrayGetElement(psaFaces, &j, &pUnkFace);
                            IFace2Ptr swFace = pUnkFace;

                            if (swFace) {
                                // 4. Get Tessellation (Triangles) for the face
                                // VARIANT_TRUE means "use high-quality tessellation"
                                variant_t vTess = swFace->GetTessTriangles(VARIANT_TRUE);
                                                            // ^Break up model to list of triangles
                                if (vTess.vt == (VT_ARRAY | VT_R4)) { // Array of floats
                                    float* pFloats = nullptr;
                                    SafeArrayAccessData(vTess.parray, (void**)&pFloats);
                                    // ^Direct memory access to array
                                    long tLower, tUpper;
                                    SafeArrayGetLBound(vTess.parray, 1, &tLower); // Start of triangle array (starts with 0)
                                    SafeArrayGetUBound(vTess.parray, 1, &tUpper); // End of triangle array

                                    long numFloats = tUpper - tLower + 1; // Total number of triangles
                                    // Each triangle has 3 vertices * 3 coordinates (x,y,z) = 9 floats
                                    for (long k = 0; k < numFloats; k += 9) {
                                        Triangle tri;
                                        tri.v1 = { (double)pFloats[k],   (double)pFloats[k+1], (double)pFloats[k+2] };
                                        tri.v2 = { (double)pFloats[k+3], (double)pFloats[k+4], (double)pFloats[k+5] };
                                        tri.v3 = { (double)pFloats[k+6], (double)pFloats[k+7], (double)pFloats[k+8] };
                                        allTriangles.push_back(tri);
                                    }

                                    SafeArrayUnaccessData(vTess.parray); // Unlock memory from Direct Access
                                }
                            }
                            if (pUnkFace) pUnkFace->Release();
                        }
                    }
                }
                if (pUnk) pUnk->Release();
            }
        }

        // 5. Write to OBJ file
        std::ofstream outFile(outputPath);
        if (!outFile.is_open()) return false;

        outFile << "# Generated by SolidWorks C++ Converter" << std::endl;

        // In OBJ, we first list all vertices, then all faces
        // To keep it simple, we'll write 3 vertices and then 1 face per triangle
        int vertexIndex = 1;
        for (const auto& tri : allTriangles) {
            outFile << "v " << tri.v1.x << " " << tri.v1.y << " " << tri.v1.z << std::endl;
            outFile << "v " << tri.v2.x << " " << tri.v2.y << " " << tri.v2.z << std::endl;
            outFile << "v " << tri.v3.x << " " << tri.v3.y << " " << tri.v3.z << std::endl;

            // f index1 index2 index3
            outFile << "f " << vertexIndex << " " << vertexIndex + 1 << " " << vertexIndex + 2 << std::endl;
            vertexIndex += 3;
        }

        std::cout << "Successfully exported " << allTriangles.size() << " triangles to " << outputPath << std::endl;
        return true;

    } catch (_com_error& e) {
        std::wcerr << L"Conversion Error: " << e.ErrorMessage() << std::endl;
        return false;
    }
}
