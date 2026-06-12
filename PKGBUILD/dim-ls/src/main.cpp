#include <filesystem>
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>
#include <sys/ioctl.h>
#include <unistd.h>

namespace fs = std::filesystem;

static int get_terminal_width() {
    struct winsize w;
    ioctl(STDOUT_FILENO, TIOCGWINSZ, &w);
    return w.ws_col ? w.ws_col : 80;
}

int main(int argc, char* argv[]) {
    fs::path dir = (argc > 1) ? argv[1] : ".";

    std::error_code ec;
    if (!fs::exists(dir, ec) || !fs::is_directory(dir, ec)) {
        std::cerr << "ls: cannot access '" << dir.string() << "'\n";
        return 1;
    }

    std::vector<std::string> files;

    for (const auto& entry : fs::directory_iterator(dir)) {
        std::string name = entry.path().filename().string();

        // скрытые файлы как в ls (по умолчанию показываем, но можно расширить позже -a)
        files.push_back(name + (entry.is_directory() ? "/" : ""));
    }

    std::sort(files.begin(), files.end());

    int width = get_terminal_width();
    int max_len = 0;
    for (auto& f : files)
        max_len = std::max(max_len, (int)f.size());

    int col_width = max_len + 2;
    int cols = std::max(1, width / col_width);

    for (size_t i = 0; i < files.size(); i++) {
        std::cout << std::left << std::setw(col_width) << files[i];
        if ((i + 1) % cols == 0)
            std::cout << '\n';
    }

    if (!files.empty() && files.size() % cols != 0)
        std::cout << '\n';

    return 0;
}
