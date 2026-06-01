#include "SolidWorksEngine.h"
#include <iostream>
#include <string>

int main() {
    // Initialize COM
    HRESULT hr = CoInitializeEx(NULL, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return 1;
    }

    std::cout << "--- SolidWorks Engine Test ---" << std::endl;

    {
        // 2. Create the engine
        SolidWorksEngine engine;

        // 3. Ask for the path manually
        std::string testPath;
        std::cout << "Please enter the FULL path to your .SLDPRT file: " << std::endl;

        // We use getline instead of cin >> because paths often have spaces
        std::getline(std::cin, testPath);
        // Remove quotes if the user copied path as "C:\path"
        if (!testPath.empty() && testPath.front() == '"') testPath.erase(0, 1);
        if (!testPath.empty() && testPath.back() == '"') testPath.pop_back();

        if (engine.OpenPart(testPath)) {
            std::cout << "SUCCESS: Part opened!" << std::endl;
            engine.ConvertToObj("test.obj");
            
            std::cout << "Press Enter to close..." << std::endl;
            std::cin.get();
            
            engine.ClosePart();
        } else {
            std::cout << "FAILURE: Could not open part." << std::endl;
        }
    }

    // 4. Uninitialize COM when the program ends
    CoUninitialize();
    return 0;
}
