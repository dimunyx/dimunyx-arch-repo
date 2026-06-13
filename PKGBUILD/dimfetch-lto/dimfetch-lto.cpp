#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <algorithm>
#include <unistd.h>
#include <sys/utsname.h>
#include <sys/sysinfo.h>
#include <pwd.h>

// ANSI color codes
struct Color {
    static constexpr const char* reset  = "\033[0m";
    static constexpr const char* bold   = "\033[1m";
    static constexpr const char* red    = "\033[31m";
    static constexpr const char* green  = "\033[32m";
    static constexpr const char* yellow = "\033[33m";
    static constexpr const char* blue   = "\033[34m";
    static constexpr const char* purple = "\033[35m";
    static constexpr const char* cyan   = "\033[36m";
    static constexpr const char* white  = "\033[37m";
    static constexpr const char* label  = "\033[38;2;137;180;250m";
};

std::string exec(const char* cmd) {
    std::string result;
    FILE* pipe = popen(cmd, "r");
    if (!pipe) return "?";
    char buf[128];
    while (fgets(buf, sizeof(buf), pipe))
        result += buf;
    pclose(pipe);
    if (!result.empty() && result.back() == '\n')
        result.pop_back();
    return result;
}

std::string read_file_line(const std::string& path) {
    std::ifstream f(path);
    std::string line;
    if (f.good()) {
        std::getline(f, line);
        return line;
    }
    return "";
}

std::string get_value_after_colon(const std::string& path, const std::string& key) {
    std::ifstream f(path);
    std::string line;
    while (std::getline(f, line)) {
        auto pos = line.find(':');
        if (pos != std::string::npos) {
            std::string k = line.substr(0, pos);
            k.erase(k.find_last_not_of(" \t") + 1);
            if (k == key) {
                std::string v = line.substr(pos + 1);
                v.erase(0, v.find_first_not_of(" \t"));
                return v;
            }
        }
    }
    return "";
}

std::string get_os() {
    std::ifstream f("/etc/os-release");
    if (!f.good()) return "Linux";
    std::string line;
    while (std::getline(f, line)) {
        if (line.find("PRETTY_NAME=") == 0) {
            auto v = line.substr(12);
            if (v.front() == '"' && v.back() == '"')
                v = v.substr(1, v.size() - 2);
            if (v == "Arch Linux") {
                struct utsname u;
                uname(&u);
                v += std::string(" ") + u.machine;
            }
            return v;
        }
    }
    return "Linux";
}

std::string get_hostname() {
    char host[256] = {};
    gethostname(host, sizeof(host));
    return host;
}

std::string get_username() {
    auto pw = getpwuid(getuid());
    return pw ? pw->pw_name : "user";
}

std::string get_kernel() {
    struct utsname u;
    uname(&u);
    return u.release;
}

std::string get_shell() {
    auto shell = getenv("SHELL");
    if (!shell) return "?";
    std::string s = shell;
    auto pos = s.rfind('/');
    if (pos != std::string::npos)
        s = s.substr(pos + 1);
    return s;
}

std::string get_de() {
    auto de = getenv("XDG_CURRENT_DESKTOP");
    if (!de) {
        de = getenv("DESKTOP_SESSION");
        if (!de) return "?";
    }
    return de;
}

std::string get_packages() {
    auto dpkg = exec("dpkg --list 2>/dev/null | wc -l");
    if (dpkg != "?" && dpkg != "0") {
        int n = std::stoi(dpkg) - 5; // header lines
        return std::to_string(n) + " (dpkg)";
    }
    auto rpm = exec("rpm -qa 2>/dev/null | wc -l");
    if (rpm != "?" && rpm != "0")
        return rpm + " (rpm)";
    auto pacman = exec("pacman -Q 2>/dev/null | wc -l");
    if (pacman != "?" && pacman != "0")
        return pacman + " (pacman)";
    return "?";
}

std::string get_uptime() {
    struct sysinfo si;
    sysinfo(&si);
    long uptime = si.uptime;
    int days = uptime / 86400;
    int hours = (uptime % 86400) / 3600;
    int mins = (uptime % 3600) / 60;
    std::string result;
    if (days > 0) result += std::to_string(days) + "d ";
    if (hours > 0) result += std::to_string(hours) + "h ";
    result += std::to_string(mins) + "m";
    return result;
}

std::string get_memory() {
    auto total = get_value_after_colon("/proc/meminfo", "MemTotal");
    auto avail = get_value_after_colon("/proc/meminfo", "MemAvailable");
    if (total.empty() || avail.empty()) return "?";
    long t = std::stol(total) / 1024;
    long a = std::stol(avail) / 1024;
    return std::to_string(t - a) + "MiB / " + std::to_string(t) + "MiB";
}

std::string get_cpu() {
    std::ifstream f("/proc/cpuinfo");
    std::string line;
    std::string model;
    int cores = 0;
    while (std::getline(f, line)) {
        if (line.find("model name") == 0 && model.empty()) {
            model = line.substr(line.find(':') + 2);
        }
        if (line.find("processor") == 0)
            cores++;
    }
    if (model.empty()) return "?";
    return model + " (" + std::to_string(cores) + ")";
}

std::string get_gpu() {
    auto gpu = exec("lspci 2>/dev/null | grep -i 'vga\\|3d\\|display' | head -1 | sed 's/.*: //'");
    if (gpu == "?") return "?";
    // Simplify: remove text in parens
    auto p = gpu.find(" (");
    if (p != std::string::npos) gpu = gpu.substr(0, p);
    return gpu;
}

std::string get_disk() {
    auto usage = exec("df -h / | awk 'NR==2{print $3 \" / \" $2 \" (\" $5 \")\"}'");
    return usage.empty() ? "?" : usage;
}

std::string get_terminal() {
    auto t = getenv("TERM");
    return t ? t : "?";
}

struct DistroLogo {
    std::vector<std::string> lines;
    std::vector<const char*> colors;
};

DistroLogo get_distro_logo(const std::string& os) {
    if (os.find("Arch") != std::string::npos) {
        return {{
            "              ⣸⣇              ",
            "             ⢰⣿⣿⡆             ",
            "            ⢠⣿⣿⣿⣿⡄            ",
            "            ⢿⣿⣿⣿⣿⣿⡄           ",
            "          ⢀⣷⣤⣙⢻⣿⣿⣿⣿⡀          ",
            "         ⢀⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⡀         ",
            "        ⢠⣾⣿⣿⣿⣿⣿⣿⣿⣿⣿⣿⣷⡄        ",
            "       ⢠⣿⣿⣿⣿⣿⡿⠛⠛⠿⣿⣿⣿⣿⣿⡄       ",
            "      ⢠⣿⣿⣿⣿⣿⠏⠀⠀⠀⠀⠙⣿⣿⣿⣿⣿⡄      ",
            "     ⣰⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⢿⣿⣿⣿⣿⠿⣆     ",
            "    ⣴⣿⣿⣿⣿⣿⣿⣿⠀⠀⠀⠀⠀⠀⣿⣿⣿⣿⣿⣷⣦⡀    ",
            "  ⢀⣾⣿⣿⠿⠟⠛⠋⠉⠉⠀⠀⠀⠀⠀⠀⠉⠉⠙⠛⠻⠿⣿⣿⣷⡀  ",
            " ⣠⠟⠋⠁⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀⠀  ⠈⠙⠻⣄ ",
        }, {
            Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan, Color::cyan
        }};
    }
    return {{
        "       ____       ",
        "    __/  /_       ",
        "   / _  __/       ",
        "   \\_/  \\_        ",
        "    /  /_/        ",
        "   /_____/        ",
    }, {
        Color::green, Color::green, Color::green,
        Color::green, Color::green, Color::green
    }};
}

void print_logo() {
    auto os = get_os();
    auto dlogo = get_distro_logo(os);
    std::vector<std::string> logo;
    for (size_t i = 0; i < dlogo.lines.size(); i++)
        logo.push_back(std::string(dlogo.colors[i]) + dlogo.lines[i] + Color::reset);

    for (auto& l : logo)
        std::cout << l << "\n";
    std::cout << Color::label << "    OS: " << Color::reset << os << "\n";
}

int main(int argc, char* argv[]) {
    if (argc > 1) {
        std::string arg = argv[1];
        if (arg == "--help") {
            std::cout << "Usage: " << argv[0] << " [--help] [-v | --version]\n";
            std::cout << "A system information tool (neofetch clone)\n";
            return 0;
        }
        if (arg == "-v" || arg == "--version") {
            std::cout << "dimfetch 0.3\n";
            return 0;
        }
    }
    print_logo();
    return 0;
}
