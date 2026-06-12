#include <algorithm>
#include <cerrno>
#include <climits>
#include <cmath>
#include <cstring>
#include <ctime>
#include <dirent.h>
#include <fcntl.h>
#include <functional>
#include <getopt.h>
#include <grp.h>
#include <iostream>
#include <pwd.h>
#include <string>
#include <string_view>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <unistd.h>
#include <vector>

enum class Format { one_per_line, many_per_line, long_format, with_commas, horizontal };
enum class SortBy { name, time, size, extension, none };
enum class TimeType { mtime, ctime, atime };
enum class IndicatorStyle { none, slash, file_type, classify };

static Format format = Format::many_per_line;
static SortBy sort_by = SortBy::name;
static TimeType time_type = TimeType::mtime;
static bool sort_reverse = false;
static bool show_all = false;
static bool show_almost_all = false;
static bool long_format = false;
static bool human_readable = false;
static bool print_inode = false;
static bool print_block_size = false;
static bool recursive = false;
static bool no_group = false;
static bool no_owner = false;
static bool numeric_ids = false;
static bool color_output = false;
static bool is_tty = false;
static IndicatorStyle indicator_style = IndicatorStyle::none;
static size_t line_length = 80;
static size_t tabsize = 8;

struct FileEntry {
    std::string name;
    std::string link_target;
    struct stat st;
    bool stat_ok = false;
};

static std::vector<FileEntry> files;

static size_t inode_width = 0;
static size_t blocks_width = 0;
static size_t nlink_width = 0;
static size_t owner_width = 0;
static size_t group_width = 0;
static size_t size_width = 0;

static int exit_status = 0;

static const char *color_reset = "\033[0m";
static const char *color_dir = "\033[01;34m";
static const char *color_link = "\033[01;36m";
static const char *color_exec = "\033[01;32m";
static const char *color_fifo = "\033[33m";
static const char *color_sock = "\033[01;35m";
static const char *color_blk = "\033[01;33m";
static const char *color_chr = "\033[01;33m";
static const char *color_suid = "\033[37;41m";
static const char *color_sgid = "\033[30;43m";
static const char *color_sticky = "\033[37;44m";
static const char *color_ow = "\033[34;42m";
static const char *color_tw = "\033[30;42m";

static std::string file_type_char(mode_t mode, bool stat_ok) {
    if (stat_ok) {
        if (S_ISDIR(mode)) return "/";
        if (S_ISLNK(mode)) return "@";
        if (S_ISFIFO(mode)) return "|";
        if (S_ISSOCK(mode)) return "=";
        if (S_ISBLK(mode)) return "#";
        if (S_ISCHR(mode)) return "%";
    }
    return "";
}

static std::string classify_char(mode_t mode, bool stat_ok) {
    if (stat_ok && S_ISREG(mode) && (mode & (S_IXUSR | S_IXGRP | S_IXOTH)))
        return "*";
    return file_type_char(mode, stat_ok);
}

static std::string indicator_str(bool stat_ok, mode_t mode) {
    switch (indicator_style) {
        case IndicatorStyle::slash:
            if (stat_ok && S_ISDIR(mode)) return "/";
            return "";
        case IndicatorStyle::file_type:
            return file_type_char(mode, stat_ok);
        case IndicatorStyle::classify:
            return classify_char(mode, stat_ok);
        default:
            return "";
    }
}

static std::string color_for(mode_t mode) {
    if (!color_output) return "";
    if (S_ISDIR(mode)) {
        if (mode & S_ISVTX) {
            if (mode & S_IWOTH) return color_tw;
            return color_sticky;
        }
        if (mode & S_IWOTH) return color_ow;
        return color_dir;
    }
    if (S_ISLNK(mode)) return color_link;
    if (S_ISFIFO(mode)) return color_fifo;
    if (S_ISSOCK(mode)) return color_sock;
    if (S_ISBLK(mode)) return color_blk;
    if (S_ISCHR(mode)) return color_chr;
    if (S_ISREG(mode)) {
        if (mode & S_ISUID) return color_suid;
        if (mode & S_ISGID) return color_sgid;
        if (mode & (S_IXUSR | S_IXGRP | S_IXOTH)) return color_exec;
    }
    return "";
}

static std::string mode_string(mode_t mode) {
    char buf[11];
    buf[0] = S_ISDIR(mode) ? 'd' : S_ISLNK(mode) ? 'l' : S_ISCHR(mode) ? 'c'
               : S_ISBLK(mode) ? 'b' : S_ISFIFO(mode) ? 'p' : S_ISSOCK(mode) ? 's'
               : S_ISREG(mode) ? '-' : '?';
    buf[1] = (mode & S_IRUSR) ? 'r' : '-';
    buf[2] = (mode & S_IWUSR) ? 'w' : '-';
    buf[3] = (mode & S_IXUSR) ? ((mode & S_ISUID) ? 's' : 'x') : ((mode & S_ISUID) ? 'S' : '-');
    buf[4] = (mode & S_IRGRP) ? 'r' : '-';
    buf[5] = (mode & S_IWGRP) ? 'w' : '-';
    buf[6] = (mode & S_IXGRP) ? ((mode & S_ISGID) ? 's' : 'x') : ((mode & S_ISGID) ? 'S' : '-');
    buf[7] = (mode & S_IROTH) ? 'r' : '-';
    buf[8] = (mode & S_IWOTH) ? 'w' : '-';
    buf[9] = (mode & S_IXOTH) ? ((mode & S_ISVTX) ? 't' : 'x') : ((mode & S_ISVTX) ? 'T' : '-');
    buf[10] = '\0';
    return buf;
}

static std::string user_name(uid_t uid) {
    if (numeric_ids) return std::to_string(uid);
    struct passwd *pw = getpwuid(uid);
    return pw ? pw->pw_name : std::to_string(uid);
}

static std::string group_name(gid_t gid) {
    if (numeric_ids) return std::to_string(gid);
    struct group *gr = getgrgid(gid);
    return gr ? gr->gr_name : std::to_string(gid);
}

static std::string time_string(time_t t, TimeType tt) {
    (void)tt;
    struct tm *tm = localtime(&t);
    if (!tm) return "??????????";
    char buf[64];
    time_t now = time(nullptr);
    struct tm *tm_now = localtime(&now);
    bool recent = tm_now && (tm->tm_year == tm_now->tm_year);
    if (recent)
        strftime(buf, sizeof(buf), "%b %e %H:%M", tm);
    else
        strftime(buf, sizeof(buf), "%b %e  %Y", tm);
    return buf;
}

static std::string human_size(off_t size) {
    if (!human_readable) return std::to_string(size);
    static const char *units[] = {"", "K", "M", "G", "T", "P"};
    double s = size;
    int i = 0;
    while (std::abs(s) >= 1024 && i < 5) { s /= 1024; i++; }
    char buf[32];
    if (i == 0)
        snprintf(buf, sizeof(buf), "%lld", (long long)size);
    else if (s < 10)
        snprintf(buf, sizeof(buf), "%.1f%s", s, units[i]);
    else
        snprintf(buf, sizeof(buf), "%.0f%s", s, units[i]);
    return buf;
}

static std::string block_string(blkcnt_t blocks) {
    auto s = human_size(blocks * 512);
    return s;
}

static void init_widths() {
    inode_width = 0;
    blocks_width = 0;
    nlink_width = 0;
    owner_width = 0;
    group_width = 0;
    size_width = 0;
    for (auto &f : files) {
        if (!f.stat_ok) continue;
        auto ino_str = std::to_string(f.st.st_ino);
        inode_width = std::max(inode_width, ino_str.size());
        auto blk_str = block_string(f.st.st_blocks);
        blocks_width = std::max(blocks_width, blk_str.size());
        auto nlink_str = std::to_string(f.st.st_nlink);
        nlink_width = std::max(nlink_width, nlink_str.size());
        auto own_str = user_name(f.st.st_uid);
        owner_width = std::max(owner_width, own_str.size());
        auto grp_str = group_name(f.st.st_gid);
        if (!no_group)
            group_width = std::max(group_width, grp_str.size());
        auto sz_str = human_size(f.st.st_size);
        size_width = std::max(size_width, sz_str.size());
    }
}

static bool should_ignore(const std::string &name) {
    if (name == "." || name == "..") {
        return !show_all && !show_almost_all;
    }
    if (name[0] == '.') return !show_all;
    return false;
}

static void add_entries(const std::string &dirpath) {
    DIR *dir = opendir(dirpath.empty() ? "." : dirpath.c_str());
    if (!dir) {
        std::cerr << "dim-ls: cannot open directory '" << dirpath << "': "
                  << strerror(errno) << std::endl;
        exit_status = 1;
        return;
    }
    struct dirent *entry;
    while ((entry = readdir(dir))) {
        std::string name = entry->d_name;
        if (should_ignore(name)) continue;

        std::string full = dirpath.empty() ? name : dirpath + "/" + name;
        FileEntry fe;
        fe.name = name;

        struct stat st;
        if (lstat(full.c_str(), &st) == 0) {
            fe.st = st;
            fe.stat_ok = true;
            if (S_ISLNK(st.st_mode)) {
                char linkbuf[4096];
                ssize_t len = readlink(full.c_str(), linkbuf, sizeof(linkbuf) - 1);
                if (len != -1) {
                    linkbuf[len] = '\0';
                    fe.link_target = linkbuf;
                }
            }
        }
        files.push_back(std::move(fe));
    }
    closedir(dir);
}

static bool name_cmp(const FileEntry &a, const FileEntry &b) {
    return a.name < b.name;
}

static bool time_cmp(const FileEntry &a, const FileEntry &b) {
    struct timespec ta{0, 0}, tb{0, 0};
    if (a.stat_ok) {
        switch (time_type) {
            case TimeType::mtime: ta = a.st.st_mtim; break;
            case TimeType::ctime: ta = a.st.st_ctim; break;
            case TimeType::atime: ta = a.st.st_atim; break;
        }
    }
    if (b.stat_ok) {
        switch (time_type) {
            case TimeType::mtime: tb = b.st.st_mtim; break;
            case TimeType::ctime: tb = b.st.st_ctim; break;
            case TimeType::atime: tb = b.st.st_atim; break;
        }
    }
    if (ta.tv_sec != tb.tv_sec) return ta.tv_sec > tb.tv_sec;
    if (ta.tv_nsec != tb.tv_nsec) return ta.tv_nsec > tb.tv_nsec;
    return a.name < b.name;
}

static bool size_cmp(const FileEntry &a, const FileEntry &b) {
    off_t sa = a.stat_ok ? a.st.st_size : 0;
    off_t sb = b.stat_ok ? b.st.st_size : 0;
    if (sa != sb) return sa > sb;
    return a.name < b.name;
}

static bool ext_cmp(const FileEntry &a, const FileEntry &b) {
    auto ext = [](const std::string &s) -> std::string_view {
        auto dot = s.rfind('.');
        return dot == std::string::npos ? "" : std::string_view(s).substr(dot);
    };
    auto ea = ext(a.name), eb = ext(b.name);
    if (ea != eb) return ea < eb;
    return a.name < b.name;
}

static void sort_files() {
    if (files.empty()) return;
    switch (sort_by) {
        case SortBy::name: std::sort(files.begin(), files.end(), name_cmp); break;
        case SortBy::time: std::sort(files.begin(), files.end(), time_cmp); break;
        case SortBy::size: std::sort(files.begin(), files.end(), size_cmp); break;
        case SortBy::extension: std::sort(files.begin(), files.end(), ext_cmp); break;
        case SortBy::none: break;
    }
    if (sort_reverse && sort_by != SortBy::none)
        std::reverse(files.begin(), files.end());
}

static void print_entry_long(const FileEntry &f) {
    if (print_inode) {
        printf("%*s ", (int)inode_width, f.stat_ok ? std::to_string(f.st.st_ino).c_str() : "?");
    }
    if (print_block_size) {
        printf("%*s ", (int)blocks_width,
               f.stat_ok ? block_string(f.st.st_blocks).c_str() : "?");
    }
    if (f.stat_ok) {
        printf("%s ", mode_string(f.st.st_mode).c_str());
        printf("%*ld ", (int)nlink_width, (long)f.st.st_nlink);
        if (!no_owner)
            printf("%-*s ", (int)owner_width, user_name(f.st.st_uid).c_str());
        if (!no_group)
            printf("%-*s ", (int)group_width, group_name(f.st.st_gid).c_str());
        printf("%*s ", (int)size_width, human_size(f.st.st_size).c_str());
        printf("%s ", time_string(f.st.st_mtime, TimeType::mtime).c_str());
    } else {
        printf("?---------- ? ? %-*s ? %-*s ? %*s ? %s ",
               (int)owner_width, "?", (int)group_width, "?",
               (int)size_width, "?", "??????????");
    }
    auto col = color_for(f.st.st_mode);
    if (!col.empty()) printf("%s", col.c_str());
    printf("%s", f.name.c_str());
    if (!col.empty()) printf("%s", color_reset);
    printf("%s", indicator_str(f.stat_ok, f.st.st_mode).c_str());
    if (f.stat_ok && S_ISLNK(f.st.st_mode) && !f.link_target.empty())
        printf(" -> %s", f.link_target.c_str());
    printf("\n");
}

static void print_entry_short(const FileEntry &f, size_t col_width) {
    auto col = color_for(f.st.st_mode);
    if (!col.empty()) printf("%s", col.c_str());

    if (print_inode)
        printf("%*s ", (int)inode_width,
               f.stat_ok ? std::to_string(f.st.st_ino).c_str() : "?");
    if (print_block_size)
        printf("%*s ", (int)blocks_width,
               f.stat_ok ? block_string(f.st.st_blocks).c_str() : "?");

    printf("%s", f.name.c_str());
    if (!col.empty()) printf("%s", color_reset);
    auto ind = indicator_str(f.stat_ok, f.st.st_mode);
    printf("%s", ind.c_str());

    size_t printed = f.name.size() + ind.size();
    if (col_width > printed)
        for (size_t i = printed; i < col_width; i++)
            putchar(' ');
}

static void print_files() {
    if (files.empty()) return;

    if (format == Format::long_format) {
        blkcnt_t total = 0;
        for (auto &f : files)
            if (f.stat_ok) total += f.st.st_blocks;
        printf("total %s\n", block_string(total).c_str());
        for (auto &f : files)
            print_entry_long(f);
        return;
    }

    size_t max_name = 0;
    for (auto &f : files) {
        auto len = f.name.size() + (indicator_style != IndicatorStyle::none ? 1 : 0);
        if (print_inode && f.stat_ok)
            len += inode_width + 1;
        if (print_block_size && f.stat_ok)
            len += blocks_width + 1;
        max_name = std::max(max_name, len);
    }

    if (format == Format::one_per_line) {
        for (auto &f : files) {
            print_entry_short(f, 0);
            putchar('\n');
        }
        return;
    }

    size_t cols = line_length / (max_name + 2);
    if (cols < 1) cols = 1;
    size_t col_width = line_length / cols;
    if (col_width < max_name + 2) col_width = max_name + 2;

    if (format == Format::many_per_line) {
        size_t rows = (files.size() + cols - 1) / cols;
        for (size_t r = 0; r < rows; r++) {
            for (size_t c = 0; c < cols; c++) {
                size_t idx = c * rows + r;
                if (idx >= files.size()) continue;
                print_entry_short(files[idx], col_width);
            }
            putchar('\n');
        }
    } else if (format == Format::horizontal) {
        for (size_t i = 0; i < files.size(); i++) {
            if (i > 0) {
                putchar(' ');
                putchar(' ');
            }
            print_entry_short(files[i], col_width);
        }
        putchar('\n');
    } else if (format == Format::with_commas) {
        for (size_t i = 0; i < files.size(); i++) {
            if (i > 0) {
                size_t guess = 2;
                if (guess + files[i].name.size() + 2 > line_length)
                    printf(",\n");
                else
                    printf(", ");
            }
            print_entry_short(files[i], 0);
        }
        putchar('\n');
    }
}

static void list_dir(const std::string &path, bool top_level) {
    files.clear();
    add_entries(path);
    sort_files();
    init_widths();

    if (!top_level || recursive) {
        printf("%s:\n", path.empty() ? "." : path.c_str());
    }
    print_files();
}

struct option long_options[] = {
    {"all", no_argument, nullptr, 'a'},
    {"almost-all", no_argument, nullptr, 'A'},
    {"author", no_argument, nullptr, 256},
    {"block-size", required_argument, nullptr, 257},
    {"classify", no_argument, nullptr, 'F'},
    {"color", optional_argument, nullptr, 258},
    {"dereference", no_argument, nullptr, 'L'},
    {"directory", no_argument, nullptr, 'd'},
    {"dired", no_argument, nullptr, 'D'},
    {"escape", no_argument, nullptr, 'b'},
    {"file-type", no_argument, nullptr, 259},
    {"format", required_argument, nullptr, 260},
    {"full-time", no_argument, nullptr, 261},
    {"group-directories-first", no_argument, nullptr, 262},
    {"help", no_argument, nullptr, 263},
    {"hide", required_argument, nullptr, 264},
    {"hide-control-chars", no_argument, nullptr, 'q'},
    {"human-readable", no_argument, nullptr, 'h'},
    {"ignore", required_argument, nullptr, 'I'},
    {"ignore-backups", no_argument, nullptr, 'B'},
    {"indicator-style", required_argument, nullptr, 265},
    {"inode", no_argument, nullptr, 'i'},
    {"literal", no_argument, nullptr, 'N'},
    {"no-group", no_argument, nullptr, 'G'},
    {"numeric-uid-gid", no_argument, nullptr, 'n'},
    {"quote-name", no_argument, nullptr, 'Q'},
    {"quoting-style", required_argument, nullptr, 266},
    {"recursive", no_argument, nullptr, 'R'},
    {"reverse", no_argument, nullptr, 'r'},
    {"show-control-chars", no_argument, nullptr, 267},
    {"si", no_argument, nullptr, 268},
    {"size", no_argument, nullptr, 's'},
    {"sort", required_argument, nullptr, 269},
    {"tabsize", required_argument, nullptr, 'T'},
    {"time", required_argument, nullptr, 270},
    {"time-style", required_argument, nullptr, 271},
    {"version", no_argument, nullptr, 272},
    {"width", required_argument, nullptr, 'w'},
    {nullptr, 0, nullptr, 0}
};

static void print_usage() {
    std::cout << "Usage: dim-ls [OPTION]... [FILE]...\n"
              << "List information about the FILEs (the current directory by default).\n\n"
              << "  -a, --all                  do not ignore entries starting with .\n"
              << "  -A, --almost-all           do not list implied . and ..\n"
              << "  -B, --ignore-backups       do not list implied entries ending with ~\n"
              << "  -c                         with -lt: sort by, and show, ctime\n"
              << "                             with -l: show ctime and sort by name\n"
              << "                             otherwise: sort by ctime\n"
              << "  -C                         list entries by columns\n"
              << "      --color[=WHEN]         colorize the output\n"
              << "  -d, --directory            list directory entries instead of contents\n"
              << "  -F, --classify             append indicator (one of */=>@|) to entries\n"
              << "      --file-type            likewise, except do not append '*'\n"
              << "  -G, --no-group             in a long listing, don't print group names\n"
              << "  -h, --human-readable       with -l, print sizes in human readable format\n"
              << "  -H, --dereference-command-line\n"
              << "                             follow symbolic links listed on the command line\n"
              << "  -i, --inode                print the index number of each file\n"
              << "  -l                         use a long listing format\n"
              << "  -L, --dereference          follow symbolic links\n"
              << "  -m                         fill width with a comma separated list of entries\n"
              << "  -n, --numeric-uid-gid      like -l, but list numeric user and group IDs\n"
              << "  -N, --literal              print raw entry names\n"
              << "  -o                         like -l, but do not list group information\n"
              << "  -p, --indicator-style=slash\n"
              << "                             append / indicator to directories\n"
              << "  -q, --hide-control-chars   print ? instead of non graphic characters\n"
              << "  -r, --reverse              reverse order while sorting\n"
              << "  -R, --recursive            list subdirectories recursively\n"
              << "  -s, --size                 print the allocated size of each file, in blocks\n"
              << "  -S                         sort by file size\n"
              << "      --sort=WORD            sort by WORD instead of name: none, extension,\n"
              << "                             size, time, version\n"
              << "      --time=WORD            with -l, show time as WORD instead of modification\n"
              << "                             time: atime, access, use, ctime, status\n"
              << "  -t                         sort by modification time\n"
              << "  -T, --tabsize=COLS         assume tab stops at each COLS instead of 8\n"
              << "  -u                         with -lt: sort by, and show, access time\n"
              << "                             with -l: show access time and sort by name\n"
              << "                             otherwise: sort by access time\n"
              << "  -U                         do not sort; list entries in directory order\n"
              << "  -v                         natural sort of (version) numbers within text\n"
              << "      --version              output version information and exit\n"
              << "  -w, --width=COLS           assume screen width instead of current value\n"
              << "  -x                         list entries by lines instead of by columns\n"
              << "  -X                         sort alphabetically by entry extension\n"
              << "  -1                         list one file per line\n"
              << "      --help                 display this help and exit\n";
}

int main(int argc, char *argv[]) {
    is_tty = isatty(STDOUT_FILENO);
    if (is_tty) {
        format = Format::many_per_line;
    } else {
        format = Format::one_per_line;
    }

    if (auto *p = getenv("COLUMNS"); p && *p)
        line_length = std::stoul(p);
#ifdef TIOCGWINSZ
    struct winsize ws;
    if (ioctl(STDOUT_FILENO, TIOCGWINSZ, &ws) != -1 && ws.ws_col > 0)
        line_length = ws.ws_col;
#endif
    if (auto *p = getenv("TABSIZE"); p)
        tabsize = std::stoul(p);

    int opt;
    while ((opt = getopt_long(argc, argv, "1AaBbCcdFfGgHhiI:kLlmnNopqRrSstT:uUvxXw:Z",
                              long_options, nullptr)) != -1)
    {
        switch (opt) {
            case '1': if (format != Format::long_format) format = Format::one_per_line; break;
            case 'a': show_all = true; break;
            case 'A': show_almost_all = true; break;
            case 'B': break;
            case 'b': break;
            case 'C': format = Format::many_per_line; break;
            case 'c': time_type = TimeType::ctime; break;
            case 'd': break;
            case 'F': indicator_style = IndicatorStyle::classify; break;
            case 'f': show_all = true; sort_by = SortBy::none; break;
            case 'G': no_group = true; break;
            case 'g': long_format = true; no_owner = true; format = Format::long_format; break;
            case 'H': break;
            case 'h': human_readable = true; break;
            case 'I': break;
            case 'i': print_inode = true; break;
            case 'k': break;
            case 'L': break;
            case 'l': format = Format::long_format; long_format = true; break;
            case 'm': format = Format::with_commas; break;
            case 'N': break;
            case 'n': numeric_ids = true; format = Format::long_format; long_format = true; break;
            case 'o': no_group = true; format = Format::long_format; long_format = true; break;
            case 'p': indicator_style = IndicatorStyle::slash; break;
            case 'q': break;
            case 'r': sort_reverse = true; break;
            case 'R': recursive = true; break;
            case 'S': sort_by = SortBy::size; break;
            case 's': print_block_size = true; break;
            case 'T':
                tabsize = std::stoul(optarg);
                break;
            case 't': sort_by = SortBy::time; break;
            case 'U': sort_by = SortBy::none; break;
            case 'u': time_type = TimeType::atime; break;
            case 'v': sort_by = SortBy::name; break;
            case 'w':
                line_length = std::stoul(optarg);
                break;
            case 'x': format = Format::horizontal; break;
            case 'X': sort_by = SortBy::extension; break;
            case 'Z': break;
            case 258:
                color_output = true;
                break;
            case 259: indicator_style = IndicatorStyle::file_type; break;
            case 260:
                if (strcmp(optarg, "long") == 0 || strcmp(optarg, "verbose") == 0)
                    format = Format::long_format;
                else if (strcmp(optarg, "single-column") == 0)
                    format = Format::one_per_line;
                else if (strcmp(optarg, "vertical") == 0 || strcmp(optarg, "C") == 0)
                    format = Format::many_per_line;
                else if (strcmp(optarg, "horizontal") == 0 || strcmp(optarg, "across") == 0)
                    format = Format::horizontal;
                else if (strcmp(optarg, "commas") == 0)
                    format = Format::with_commas;
                break;
            case 263: print_usage(); return 0;
            case 265:
                if (strcmp(optarg, "none") == 0) indicator_style = IndicatorStyle::none;
                else if (strcmp(optarg, "slash") == 0) indicator_style = IndicatorStyle::slash;
                else if (strcmp(optarg, "file-type") == 0) indicator_style = IndicatorStyle::file_type;
                else if (strcmp(optarg, "classify") == 0) indicator_style = IndicatorStyle::classify;
                break;
            case 269:
                if (strcmp(optarg, "none") == 0) sort_by = SortBy::none;
                else if (strcmp(optarg, "name") == 0) sort_by = SortBy::name;
                else if (strcmp(optarg, "size") == 0) sort_by = SortBy::size;
                else if (strcmp(optarg, "time") == 0) sort_by = SortBy::time;
                else if (strcmp(optarg, "extension") == 0) sort_by = SortBy::extension;
                else if (strcmp(optarg, "version") == 0) sort_by = SortBy::name;
                break;
            case 270:
                if (strcmp(optarg, "atime") == 0 || strcmp(optarg, "access") == 0 || strcmp(optarg, "use") == 0)
                    time_type = TimeType::atime;
                else if (strcmp(optarg, "ctime") == 0 || strcmp(optarg, "status") == 0)
                    time_type = TimeType::ctime;
                break;
            case 272:
                std::cout << "dim-ls 0.0.4\n";
                return 0;
            default:
                std::cerr << "Try 'dim-ls --help' for more information.\n";
                return 2;
        }
    }

    if (format == Format::long_format || print_block_size) {
    }

    std::vector<std::string> paths;
    if (optind >= argc) {
        paths.push_back(".");
    } else {
        for (int i = optind; i < argc; i++)
            paths.push_back(argv[i]);
    }

    bool first = true;
    for (auto &p : paths) {
        struct stat st;
        if (lstat(p.c_str(), &st) == 0 && S_ISDIR(st.st_mode)) {
            if (!first) printf("\n");
            first = false;
            list_dir(p, paths.size() <= 1);
        } else {
            files.clear();
            FileEntry fe;
            fe.name = p;
            if (stat(p.c_str(), &st) == 0) {
                fe.st = st;
                fe.stat_ok = true;
            }
            files.push_back(std::move(fe));
            sort_files();
            init_widths();
            if (format == Format::long_format)
                print_entry_long(files[0]);
            else
                print_entry_short(files[0], 0), printf("\n");
        }
    }

    return exit_status;
}
