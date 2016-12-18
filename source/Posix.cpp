
namespace imajuscule {
    
    DirectoryPath Posix::user_path() {
        const char *path;

        if ((path = getenv("HOME")) == nullptr) {
            path = getpwuid(getuid())->pw_dir;
        }
        return DirectoryPath(path);
    }
}
