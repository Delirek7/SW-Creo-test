#include "SolidWorksEngine.h"
#include <iostream>
#include <string>
#include <filesystem> // For checking if file exists

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    // 1. Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "CRITICAL: COM Initialization failed." << std::endl;
        return 1;
    }

    std::cout << "--- SolidWorks Engine Test ---" << std::endl;

    try {
        // --- PHASE 1: INPUT COLLECTION ---
        std::string testPath;

        // Check Command Line Arguments
        // argv[0] is the program name, argv[1] is the first user argument
        if (argc >= 2) {
            testPath = argv[1];
        } else {
            // Fallback to manual input if no argument provided
            std::cout << "Please enter the FULL path to your .SLDPRT file: " << std::endl;
            std::getline(std::cin, testPath);
        }

        // --- PHASE 2: VALIDATION (Before launching SW) ---
        // 
        // 3. Cleanup quotes
        if (!testPath.empty() && testPath.front() == '"') testPath.erase(0, 1);
        if (!testPath.empty() && testPath.back() == '"') testPath.pop_back();

        if (testPath.empty()) throw std::runtime_error("No file path provided.");

        // 4. Execute the conversion
        // Check if file physically exists on disk
        if (!fs::exists(testPath)) {
            throw std::runtime_error("File does not exist: " + testPath);
        }

        // Check extension
        std::string ext = ".SLDPRT";
        if (testPath.length() < ext.length() ||
            testPath.compare(testPath.length() - ext.length(), ext.length(), ext) != 0) {
            throw std::runtime_error("File is not a .SLDPRT part.");
        }

        // --- PHASE 3: EXECUTION (Only now do we launch SW) ---

        std::cout << "Input validated. Connecting to SolidWorks..." << std::endl;
        SolidWorksEngine engine; // This launches SolidWorks

        engine.OpenPart(testPath);
        std::cout << "SUCCESS: Part opened!" << std::endl;

        engine.ConvertToObj("converted_model.obj");
        std::cout << "SUCCESS: Export finished!" << std::endl;

        engine.ClosePart();

    } catch (const std::exception& e) {
        // --- THE ERROR PATH ---
        // Every single error from the App, Part, Extractor, or Exporter ends up here.
        std::cerr << "\n[ERROR]: " << e.what() << std::endl;
    } catch (...) {
        std::cerr << "\n[ERROR]: An unknown fatal error occurred." << std::endl;
    }

    // 5. Clean up COM
    CoUninitialize();
    std::cout << "\nPress Enter to exit." << std::endl;
    std::cin.get();

    return 0;
}
