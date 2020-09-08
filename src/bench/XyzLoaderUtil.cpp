#include <filesystem>
#include <fstream>

static void runWithTestFiles(unsigned int numFiles, unsigned int numLines,
                             const std::function<void(const std::string &)> &func) {
    const std::string tmpDir = "dtm-" + std::to_string(numFiles) + "-" + std::to_string(numLines);
    if (std::filesystem::exists(tmpDir)) {
        std::filesystem::remove_all(tmpDir);
    }
    std::filesystem::create_directory(tmpDir);

    for (unsigned int i = 0; i < numFiles; i++) {
        std::ofstream file;
        file.open(tmpDir + "/33" + std::to_string(i + 1000000) + "_dgm20.xyz");
        for (unsigned int j = 0; j < numLines; j++) {
            file << j * 1.0F << " " << j * 1000.0F << " " << j * 1000000.0F << "\n";
        }
        file.close();
    }

    func(tmpDir);

    std::filesystem::remove_all(tmpDir);
}
