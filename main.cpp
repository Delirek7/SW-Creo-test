#include "SolidWorksEngine.h"
#include <iostream>
#include <string>
#include <filesystem>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return 1;

    std::cout << "--- SolidWorks Engine Test ---" << std::endl;

    try {
        std::string testPath;
        if (argc >= 2) testPath = argv[1];
        else {
            std::cout << "Enter .SLDPRT path: ";
            std::getline(std::cin, testPath);
        }

        // Validation
        if (!testPath.empty() && testPath.front() == '"') testPath.erase(0, 1);
        if (!testPath.empty() && testPath.back() == '"') testPath.pop_back();
        if (!fs::exists(testPath)) throw std::runtime_error("File not found.");

        SolidWorksEngine engine;

        // 1. Transition to 'Open' state
        // The 'activePart' variable now holds the State!
        auto activePart = engine.OpenPart(testPath);

        if (activePart) {
            std::cout << "State: Part is Open." << std::endl;

            // 2. Perform action on the state
            engine.ConvertToObj(*activePart, "test.obj");
        }

        // 3. Exit state (happens automatically when activePart goes out of scope)
        std::cout << "Exiting program..." << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "\n[ERROR]: " << e.what() << std::endl;
    }

    CoUninitialize();
    return 0;
}
