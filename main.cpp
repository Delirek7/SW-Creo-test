#include "SolidWorksEngine.h"
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char* argv[]) {
    // 1. Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return 1;
    }

    std::cout << "--- SolidWorks Engine Test ---" << std::endl;

    {
        SolidWorksEngine engine;
        std::string testPath;

        // 2. Check Command Line Arguments
        // argv[0] is the program name, argv[1] is the first user argument
        if (argc >= 2) {
            testPath = argv[1];
            std::cout << "Path received via Command Line: " << testPath << std::endl;
        } else {
            // Fallback to manual input if no argument provided
            std::cout << "Please enter the FULL path to your .SLDPRT file: " << std::endl;
            std::getline(std::cin, testPath);
        }

        // 3. Clean up the path (remove quotes)
        if (!testPath.empty() && testPath.front() == '"') testPath.erase(0, 1);
        if (!testPath.empty() && testPath.back() == '"') testPath.pop_back();

        // 4. Execute the conversion
        if (!testPath.empty()) {
            std::cout << "Processing: " << testPath << "..." << std::endl;
            
            if (engine.OpenPart(testPath)) {
                std::cout << "SUCCESS: Part opened!" << std::endl;

                // Create an output name based on the input (simple version)
                std::string outputPath = "converted_model.obj";

                if (engine.ConvertToObj(outputPath)) {
                    std::cout << "SUCCESS: Model exported to " << outputPath << std::endl;
                }

                engine.ClosePart();
            } else {
                std::cout << "FAILURE: Could not open part. Check the path and SolidWorks installation." << std::endl;
            }
        }
    }

    // 5. Clean up COM
    CoUninitialize();

    std::cout << "Done. Press Enter to exit." << std::endl;
    std::cin.get();

    return 0;
}
