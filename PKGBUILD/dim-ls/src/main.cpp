#include <filesystem>
#include <iostream>

namespace fs = std::filesystem;

int main(int argc, char* argv[]) {
    fs::path dir = argc > 1 ? argv[1] : ".";
    for (const auto& entry : fs::directory_iterator(dir))
        std::cout << entry.path().filename().string()
                  << (entry.is_directory() ? "/" : "") << '\n';
    return 0;
}
