#pragma once

#include "Windows.h"
#include "filesystem"
#include "godot_cpp/classes/image.hpp"
#include "godot_cpp/classes/input_event.hpp"
#include "godot_cpp/classes/object.hpp"
#include "libretro.h"
#include "yaml-cpp/yaml.h"

class RetroHost : public godot::Object
{
    GDCLASS( RetroHost, godot::Object )

public:
    godot::String cwd;
    static RetroHost *get_singleton();

    RetroHost();
    ~RetroHost();

    bool load_core( godot::String path );
    void unload_core();
    void run();
    void forwarded_input( const godot::Ref<godot::InputEvent> &event );

private:
    static RetroHost *singleton;

    godot::Ref<godot::Image> frame_buffer;
    godot::Ref<godot::Image> get_frame_buffer()
    {
        return frame_buffer;
    }

    // We're passing character arrays to the core, we don't know the required lifetime, so we're
    // freeing everything on unload
    std::vector<char *> please_free_me_str;

    YAML::Node core_variables;
    godot::String core_name;

    void load_core_variables();
    void save_core_variables();
    bool RetroHost::get_variable( retro_variable *variable );

    bool core_environment( unsigned cmd, void *data );

    void core_video_init( const struct retro_game_geometry *geometry );
    void core_video_refresh( const void *data, unsigned width, unsigned height, size_t pitch );
    bool core_video_set_pixel_format( unsigned format );
    godot::Image::Format pixel_format;

    void core_input_poll( void );
    int16_t core_input_state( unsigned port, unsigned device, unsigned index, unsigned id );

    void core_audio_init( retro_system_av_info av );
    void core_audio_sample( int16_t left, int16_t right );
    size_t core_audio_sample_batch( const int16_t *data, size_t frames );

    struct VFS
    {
        uint32_t supported_interface_version = 0;

        // Called on RetroHost's constructor
        void init_vfs_interface();
        struct retro_vfs_interface vfs_interface;

        // /* VFS API v1 */
        /* Get path from opaque handle. Returns the exact same path passed to file_open when getting
         * the handle Introduced in VFS API v1 */
        const char *get_path( retro_vfs_file_handle *stream );
        /* Open a file for reading or writing. If path points to a directory, this will
         * fail. Returns the opaque file handle, or NULL for error.
         * Introduced in VFS API v1 */
        struct retro_vfs_file_handle *open( const char *path, unsigned mode, unsigned hints );
        /* Close the file and release its resources. Must be called if open_file returns non-NULL.
         * Returns 0 on success, -1 on failure. Whether the call succeeds ot not, the handle passed
         * as parameter becomes invalid and should no longer be used. Introduced in VFS API v1 */
        int close( retro_vfs_file_handle *stream );
        /* Return the size of the file in bytes, or -1 for error.
         * Introduced in VFS API v1 */
        int64_t size( struct retro_vfs_file_handle *stream );
        /* Get the current read / write position for the file. Returns -1 for error.
         * Introduced in VFS API v1 */
        int64_t tell( struct retro_vfs_file_handle *stream );
        /* Set the current read/write position for the file. Returns the new position, -1 for error.
         * Introduced in VFS API v1 */
        int64_t seek( struct retro_vfs_file_handle *stream, int64_t offset, int seek_position );
        /* Read data from a file. Returns the number of bytes read, or -1 for error.
         * Introduced in VFS API v1 */
        int64_t read( struct retro_vfs_file_handle *stream, void *s, uint64_t len );
        /* Write data to a file. Returns the number of bytes written, or -1 for error.
         * Introduced in VFS API v1 */
        int64_t write( struct retro_vfs_file_handle *stream, const void *s, uint64_t len );
        /* Flush pending writes to file, if using buffered IO. Returns 0 on sucess, or -1 on
         * failure. Introduced in VFS API v1 */
        int flush( struct retro_vfs_file_handle *stream );
        /* Delete the specified file. Returns 0 on success, -1 on failure
         * Introduced in VFS API v1 */
        int remove( const char *path );
        /* Rename the specified file. Returns 0 on success, -1 on failure
         * Introduced in VFS API v1 */
        int rename( const char *old_path, const char *new_path );
        // /* VFS API v2 */
        /* Truncate file to specified size. Returns 0 on success or -1 on error
         * Introduced in VFS API v2 */
        int64_t truncate( struct retro_vfs_file_handle *stream, int64_t length );
        // /* VFS API v3 */
        /* Stat the specified file. Retruns a bitmask of RETRO_VFS_STAT_* flags, none are set if
         * path was not valid. Additionally stores file size in given variable, unless NULL is
         * given. Introduced in VFS API v3 */
        int stat( const char *path, int32_t *size );
        /* Create the specified directory. Returns 0 on success, -1 on unknown failure, -2 if
         * already exists. Introduced in VFS API v3 */
        int mkdir( const char *dir );
        /* Open the specified directory for listing. Returns the opaque dir handle, or NULL for
         * error. Support for the include_hidden argument may vary depending on the platform.
         * Introduced in VFS API v3 */
        struct retro_vfs_dir_handle *opendir( const char *dir, bool include_hidden_files );
        /* Read the directory entry at the current position, and move the read pointer to the next
         * position. Returns true on success, false if already on the last entry. Introduced in VFS
         * API v3 */
        bool read_dir( struct retro_vfs_dir_handle *dir_stream );
        /* Get the name of the last entry read. Returns a string on success, or NULL for error.
         * The returned string pointer is valid until the next call to readdir or closedir.
         * Introduced in VFS API v3 */
        const char *dirent_get_name( struct retro_vfs_dir_handle *dir_stream );
        /* Check if the last entry read was a directory. Returns true if it was, false otherwise (or
         * on error). Introduced in VFS API v3 */
        bool dirent_is_dir( struct retro_vfs_dir_handle *dir_stream );
        /* Close the directory and release its resources. Must be called if opendir returns
         * non-NULL. Returns 0 on success, -1 on failure. Whether the call succeeds ot not, the
         * handle passed as parameter becomes invalid and should no longer be used. Introduced in
         * VFS API v3 */
        int closedir( struct retro_vfs_dir_handle *dir_stream );
    } vfs;

    struct
    {
        HINSTANCE handle;
        bool initialized = false;

        void ( *retro_init )( void );
        void ( *retro_deinit )( void );
        unsigned ( *retro_api_version )( void );
        void ( *retro_get_system_info )( struct retro_system_info *info );
        void ( *retro_get_system_av_info )( struct retro_system_av_info *info );
        void ( *retro_set_controller_port_device )( unsigned port, unsigned device );
        void ( *retro_reset )( void );
        void ( *retro_run )( void );
        // size_t retro_serialize_size(void);
        // bool retro_serialize(void *data, size_t size);
        // bool retro_unserialize(const void *data, size_t size);
        // void retro_cheat_reset(void);
        // void retro_cheat_set(unsigned index, bool enabled, const char *code);
        bool ( *retro_load_game )( const struct retro_game_info *game );
        // bool retro_load_game_special(
        //   unsigned game_type,
        //   const struct retro_game_info *info,
        //   size_t num_info);
        void ( *retro_unload_game )( void );
        // unsigned retro_get_region(void);
        // void *retro_get_memory_data(unsigned id);
        // size_t retro_get_memory_size(unsigned id);

        retro_keyboard_event_t retro_keyboard_event_callback;
    } core;

protected:
    static void _bind_methods();
};