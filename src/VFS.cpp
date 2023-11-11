#include "RetroHost.hpp"

void RetroHost::VFS::init_vfs_interface() {
    // Cpp crimes again
    this->vfs_interface.get_path = [](struct retro_vfs_file_handle *stream) -> const char* {
        return RetroHost::get_singleton()->vfs.get_path(stream);
    };
    this->vfs_interface.open = [](const char *path, unsigned mode, unsigned hints) -> struct retro_vfs_file_handle* {
        return RetroHost::get_singleton()->vfs.open(path, mode, hints);
    };
    this->vfs_interface.close = [](struct retro_vfs_file_handle *stream) -> int {
        return RetroHost::get_singleton()->vfs.close(stream);
    };
    this->vfs_interface.size = [](struct retro_vfs_file_handle *stream) -> int64_t {
        return RetroHost::get_singleton()->vfs.size(stream);
    };
    this->vfs_interface.tell = [](struct retro_vfs_file_handle *stream) -> int64_t {
        return RetroHost::get_singleton()->vfs.tell(stream);
    };
    this->vfs_interface.seek = [](struct retro_vfs_file_handle *stream, int64_t offset, int seek_position) -> int64_t {
        return RetroHost::get_singleton()->vfs.seek(stream, offset, seek_position);
    };
    this->vfs_interface.read = [](struct retro_vfs_file_handle *stream, void *s, uint64_t len) -> int64_t {
        return RetroHost::get_singleton()->vfs.read(stream, s, len);
    };
    this->vfs_interface.write = [](struct retro_vfs_file_handle *stream, const void *s, uint64_t len) -> int64_t {
        return RetroHost::get_singleton()->vfs.write(stream, s, len);
    };
    this->vfs_interface.flush = [](struct retro_vfs_file_handle *stream) -> int {
        return RetroHost::get_singleton()->vfs.flush(stream);
    };
    this->vfs_interface.remove = [](const char *path) -> int {
        return RetroHost::get_singleton()->vfs.remove(path);
    };
    this->vfs_interface.rename = [](const char *old_path, const char *new_path) -> int {
        return RetroHost::get_singleton()->vfs.rename(old_path, new_path);
    };
    this->vfs_interface.truncate = [](struct retro_vfs_file_handle *stream, int64_t length) -> int64_t {
        return RetroHost::get_singleton()->vfs.truncate(stream, length);
    };
    this->vfs_interface.stat = [](const char *path, int32_t *size) -> int {
        return RetroHost::get_singleton()->vfs.stat(path, size);
    };
    this->vfs_interface.mkdir = [](const char *dir) -> int {
        return RetroHost::get_singleton()->vfs.mkdir(dir);
    };
    this->vfs_interface.opendir = [](const char *dir, bool include_hidden_files) -> struct retro_vfs_dir_handle* {
        return RetroHost::get_singleton()->vfs.opendir(dir, include_hidden_files);
    };
    this->vfs_interface.readdir = [](struct retro_vfs_dir_handle *dir_stream) -> bool {
        return RetroHost::get_singleton()->vfs.read_dir(dir_stream);
    };
    this->vfs_interface.dirent_get_name = [](struct retro_vfs_dir_handle *dir_stream) -> const char* {
        return RetroHost::get_singleton()->vfs.dirent_get_name(dir_stream);
    };
    this->vfs_interface.dirent_is_dir = [](struct retro_vfs_dir_handle *dir_stream) -> bool {
        return RetroHost::get_singleton()->vfs.dirent_is_dir(dir_stream);
    };
    this->vfs_interface.closedir = [](struct retro_vfs_dir_handle *dir_stream) -> int {
        return RetroHost::get_singleton()->vfs.closedir(dir_stream);
    };
}

// V1

const char* RetroHost::VFS::get_path(struct retro_vfs_file_handle *stream)
{
    return NULL;
}

struct retro_vfs_file_handle* RetroHost::VFS::open(const char *path, unsigned mode, unsigned hints)
{
    return NULL;
}

int RetroHost::VFS::close(struct retro_vfs_file_handle *stream)
{
    return -1;
}

int64_t RetroHost::VFS::size(struct retro_vfs_file_handle *stream)
{
    return -1;
}

int64_t RetroHost::VFS::tell(struct retro_vfs_file_handle *stream)
{
    return -1;
}

int64_t RetroHost::VFS::seek(struct retro_vfs_file_handle *stream, int64_t offset, int seek_position)
{
    return -1;
}

int64_t RetroHost::VFS::read(struct retro_vfs_file_handle *stream, void *s, uint64_t len)
{
    return -1;
}

int64_t RetroHost::VFS::write(struct retro_vfs_file_handle *stream, const void *s, uint64_t len)
{
    return -1;
}

int RetroHost::VFS::flush(struct retro_vfs_file_handle *stream)
{
    return -1;
}

int RetroHost::VFS::remove(const char *path)
{
    return -1;
}

int RetroHost::VFS::rename(const char *old_path, const char *new_path)
{
    return -1;
}

// V2

int64_t RetroHost::VFS::truncate(struct retro_vfs_file_handle *stream, int64_t length)
{
    return -1;
}

// V3

int RetroHost::VFS::stat(const char *path, int32_t *size)
{
    return -1;
}

int RetroHost::VFS::mkdir(const char *dir)
{
    return -1;
}

struct retro_vfs_dir_handle* RetroHost::VFS::opendir(const char *dir, bool include_hidden_files)
{
    return NULL;
}

bool RetroHost::VFS::read_dir(struct retro_vfs_dir_handle *dir_stream)
{
    return false;
}

const char* RetroHost::VFS::dirent_get_name(struct retro_vfs_dir_handle *dir_stream)
{
    return NULL;
}

bool RetroHost::VFS::dirent_is_dir(struct retro_vfs_dir_handle *dir_stream)
{
    return false;
}

int RetroHost::VFS::closedir(struct retro_vfs_dir_handle *dir_stream)
{
    return -1;
}