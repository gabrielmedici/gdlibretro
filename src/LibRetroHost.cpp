#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/rendering_server.hpp"

#include "LibRetroHost.h"
#include "Version.h"
#include "Windows.h"

unsigned LibRetroHost::width = 0;
unsigned LibRetroHost::height = 0;
struct retro_audio_callback LibRetroHost::audio_callback;
unsigned LibRetroHost::pixel_format;
godot::Ref<godot::Image> LibRetroHost::frame_buffer;

LibRetroHost::LibRetroHost()
{
    godot::UtilityFunctions::print( "LibRetroHost::LibRetroHost()" );
}

LibRetroHost::~LibRetroHost()
{
    godot::UtilityFunctions::print( "LibRetroHost::~LibRetroHost()" );
    unload_core();
}

static void core_log( enum retro_log_level level, const char *fmt, ... )
{
    char buffer[4096] = { 0 };
    static const char *levelstr[] = { "dbg", "inf", "wrn", "err" };
    va_list va;

    va_start( va, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, va );
    va_end( va );

    godot::UtilityFunctions::print( "[" + godot::String( levelstr[level - 1] ) + "] " + buffer );
}

bool LibRetroHost::set_pixel_format( unsigned format )
{
    pixel_format = format;
    switch ( format )
    {
        case RETRO_PIXEL_FORMAT_0RGB1555:
            godot::UtilityFunctions::print( "Pixel format: 0RGB1555" );
            return true;
        case RETRO_PIXEL_FORMAT_XRGB8888:
            godot::UtilityFunctions::print( "Pixel format: XRGB8888" );
            return true;
        case RETRO_PIXEL_FORMAT_RGB565:
            godot::UtilityFunctions::print( "Pixel format: RGB565" );
            return true;
        default:
            return false;
    }
}

bool LibRetroHost::core_environment( unsigned cmd, void *data )
{
    switch ( cmd )
    {
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        {
            core_log( RETRO_LOG_DEBUG, "[noarch] RETRO_ENVIRONMENT_GET_LOG_INTERFACE" );
            struct retro_log_callback *cb = (struct retro_log_callback *)data;
            cb->log = core_log;
        }
        break;

        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        {
            bool *bval = (bool *)data; // NOLINT
            *bval = true;
        }
        break;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        {
            const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

            if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
                return false;

            return set_pixel_format(*fmt);
        }

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
        {
            *(const char **)data = "D:\\Documents\\Code\\noarch";
            return true;
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE:
        {
            struct retro_variable *var = (struct retro_variable *)data;
            if(strcmp(var->key, "vice_sound_sample_rate") == 0) {
              var->value = "48000";
              core_log( RETRO_LOG_DEBUG, "[noarch] Set %s to %s\n",
                      var->key, var->value );
              return true;
            } else {
              core_log( RETRO_LOG_DEBUG, "[noarch] Unhandled RETRO_ENVIRONMENT_GET_VARIABLE: %s\n",
                      var->key );
            }
            return false;
        }

        case RETRO_ENVIRONMENT_SET_MESSAGE:
        {
            const struct retro_message *message = (const struct retro_message *)data;
            core_log( RETRO_LOG_DEBUG, "[noarch] RETRO_ENVIRONMENT_SET_MESSAGE: %s\n",
                      message->msg );
            break;
        }

        case RETRO_ENVIRONMENT_SHUTDOWN:
        {
            core_log( RETRO_LOG_DEBUG, "[noarch] RETRO_ENVIRONMENT_SHUTDOWN" );
            break;
        }

        case RETRO_ENVIRONMENT_SET_AUDIO_CALLBACK: {
            struct retro_audio_callback *audio_cb = (struct retro_audio_callback*)data;
            audio_callback = *audio_cb;
            return true;
        }

        default:
        {
            core_log( RETRO_LOG_DEBUG, "[noarch] Unhandled env #%u", cmd );
            return false;
        }
    }

    return true;
}

void LibRetroHost::core_video_refresh( const void *data, unsigned vwidth, unsigned vheight,
                                       size_t pitch )
{
    //godot::UtilityFunctions::print( "Video: ", vwidth, "x", vheight, " ", pitch );
    if(!data || frame_buffer.is_null() || !frame_buffer.is_valid())
        return;

    if((unsigned)frame_buffer->get_width() != vwidth || (unsigned)frame_buffer->get_height() != vheight) {
        godot::UtilityFunctions::print( "Resizing frame buffer to ", vwidth, "x", vheight );
        auto created_frame_buffer = godot::Image::create(vwidth, vheight, false, frame_buffer->get_format());
        if(created_frame_buffer.is_null() || !created_frame_buffer.is_valid()) {
            godot::UtilityFunctions::printerr( "Failed to recreate frame buffer" );
            return;
        }
        frame_buffer = created_frame_buffer;
    }

    unsigned buffer_size;
    switch (frame_buffer->get_format()) {
        case godot::Image::FORMAT_RGB565:
            buffer_size = vwidth * vheight * 2;
            break;
        case godot::Image::FORMAT_RGBA8:
            buffer_size = vwidth * vheight * 4;
            break;
        default:
            godot::UtilityFunctions::printerr( "Unhandled pixel format: ", frame_buffer->get_format() );
            return;
    }

    godot::PackedByteArray intermediary_buffer;
    intermediary_buffer.resize(buffer_size);
    memcpy((void*)intermediary_buffer.ptr(), data, buffer_size);

    frame_buffer->set_data(vwidth, vheight, false, frame_buffer->get_format(), intermediary_buffer);
}

void LibRetroHost::core_input_poll( void )
{
    //godot::UtilityFunctions::print( "Input: poll" );
}

int16_t LibRetroHost::core_input_state( unsigned port, unsigned device, unsigned index,
                                        unsigned id )
{
    //godot::UtilityFunctions::print( "Input: ", port, " ", device, " ", index, " ", id );
    (void)port;
    (void)device;
    (void)index;
    (void)id;
    return 0;
}

void LibRetroHost::core_audio_sample( int16_t left, int16_t right )
{
    //godot::UtilityFunctions::print( "Audio: ", left, " ", right );
    (void)left;
    (void)right;
}

size_t LibRetroHost::core_audio_sample_batch( const int16_t *data, size_t frames )
{
    //godot::UtilityFunctions::print( "Audio: ", frames, " frames" );
    (void)data;
    (void)frames;
    return 0;
}

void LibRetroHost::video_configure( const struct retro_game_geometry *geom )
{
    width = geom->base_width;
    height = geom->base_height;
    godot::UtilityFunctions::print( "Video cfg: ", width, "x", height );
    godot::Image::Format fmt;

    switch(pixel_format) {
      case RETRO_PIXEL_FORMAT_0RGB1555:
        fmt = godot::Image::FORMAT_RGB565;
        break;
      case RETRO_PIXEL_FORMAT_XRGB8888:
        fmt = godot::Image::FORMAT_RGBA8;
        break;
      case RETRO_PIXEL_FORMAT_RGB565:
        fmt = godot::Image::FORMAT_RGB565;
        break;
      default:
        godot::UtilityFunctions::printerr( "Unhandled pixel format: ", pixel_format );
        return;
    }

    frame_buffer = godot::Image::create(width, height, false, fmt);
}

void LibRetroHost::video_deinit()
{
}

void LibRetroHost::audio_init( retro_system_av_info av )
{
    // Print eveything in the av struct related to audio
    godot::UtilityFunctions::print( "Audio: ", av.timing.sample_rate, "Hz, ",
                                    av.timing.sample_rate / av.timing.fps, " samples per frame, ",
                                    av.timing.sample_rate / av.timing.fps / 60, " samples per second" );
    if(audio_callback.set_state) {
      audio_callback.set_state(true);
    }
}

void LibRetroHost::audio_deinit()
{
}

bool LibRetroHost::core_load_game( const char *filename )
{
    struct retro_system_timing timing = { 60.0f, 10000.0f };
    struct retro_game_geometry geom = { 100, 100, 100, 100, 1.0f };
    struct retro_system_av_info av = { geom, timing };
    struct retro_system_info system;
    g_retro.retro_get_system_info( &system );

    struct retro_game_info info = { filename, 0, 0, NULL };

    FILE *file = NULL;
    if ( filename )
    {
        fopen_s( &file, filename, "rb" );

        if ( !file )
        {
            godot::UtilityFunctions::printerr( "Error: Failed to load content from '", filename,
                                               "'" );
            fclose( file );
            return false;
        }

        fseek( file, 0, SEEK_END );
        info.size = ftell( file );
        rewind( file );
    }

    if ( filename && !system.need_fullpath )
    {
        info.data = malloc( info.size );

        if ( !info.data || !fread( (void *)info.data, info.size, 1, file ) )
        { // NOLINT
            godot::UtilityFunctions::printerr( "Error: Failed to load game data." );
            fclose( file );
            return false;
        }
        fclose( file );
    }

    if ( !g_retro.retro_load_game( NULL ) )
    {
        godot::UtilityFunctions::printerr( "Error: The core failed to load the game." );
        return false;
    }

    g_retro.retro_get_system_av_info( &av );
    godot::UtilityFunctions::print( "Video: ", av.geometry.base_width, "x",
                                    av.geometry.base_height );

    video_configure( &av.geometry );
    audio_init( av );
    return true;
}

// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    // Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if ( errorMessageID == 0 )
    {
        return std::string(); // No error message has been recorded
    }

    LPSTR messageBuffer = nullptr;

    // Ask Win32 to give us the string version of that message ID.
    // The parameters we pass in, tell Win32 to create the buffer that holds the message for us
    // (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA( FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM |
                                      FORMAT_MESSAGE_IGNORE_INSERTS,
                                  NULL, errorMessageID, MAKELANGID( LANG_NEUTRAL, SUBLANG_DEFAULT ),
                                  (LPSTR)&messageBuffer, 0, NULL );

    // Copy the error message into a std::string.
    std::string message( messageBuffer, size );

    // Free the Win32's string's buffer.
    LocalFree( messageBuffer );

    return message;
}

#define load_symbol_return_false_onerr( handle, dest, sym )                                        \
    godot::UtilityFunctions::print( "Loading core symbol \"", #sym, "\"" );                        \
    dest = (decltype( dest ))GetProcAddress( handle, #sym );                                       \
    if ( dest == NULL )                                                                            \
    {                                                                                              \
        godot::UtilityFunctions::printerr( "Could not load symbol \"", #sym,                       \
                                           "\": ", GetLastErrorAsString().c_str() );               \
        return false;                                                                              \
    }

#define def_and_load_fn_symbol_return_false_onerr( handle, dest, sym )                             \
    godot::UtilityFunctions::print( "Loading core symbol \"", #sym, "\"" );                        \
    decltype( sym ) *dest = reinterpret_cast<decltype( sym ) *>( GetProcAddress( handle, #sym ) ); \
    if ( dest == nullptr )                                                                         \
    {                                                                                              \
        godot::UtilityFunctions::printerr( "Could not load symbol \"", #sym,                       \
                                           "\": ", GetLastErrorAsString().c_str() );               \
        return false;                                                                              \
    }

#define load_retro_symbol_return_false_onerr( sym )                                                \
    load_symbol_return_false_onerr( g_retro.handle, g_retro.sym, sym )

bool LibRetroHost::load_core( godot::String path )
{
    unload_core();

    godot::UtilityFunctions::print( "Loading core: ", path );
    g_retro.handle = LoadLibrary( (const char *)path.to_utf8_buffer().ptr() );

    if ( g_retro.handle == NULL )
    {
        godot::UtilityFunctions::printerr( "Could not load DLL: ", GetLastErrorAsString().c_str() );
        return false;
    }

    load_retro_symbol_return_false_onerr( retro_init );
    load_retro_symbol_return_false_onerr( retro_deinit );
    load_retro_symbol_return_false_onerr( retro_api_version );
    load_retro_symbol_return_false_onerr( retro_get_system_info );
    load_retro_symbol_return_false_onerr( retro_get_system_av_info );
    load_retro_symbol_return_false_onerr( retro_set_controller_port_device );
    load_retro_symbol_return_false_onerr( retro_reset );
    load_retro_symbol_return_false_onerr( retro_run );
    load_retro_symbol_return_false_onerr( retro_load_game );
    load_retro_symbol_return_false_onerr( retro_unload_game );

    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_environment,
                                               retro_set_environment );
    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_video_refresh,
                                               retro_set_video_refresh );
    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_input_poll,
                                               retro_set_input_poll );
    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_input_state,
                                               retro_set_input_state );
    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_audio_sample,
                                               retro_set_audio_sample );
    def_and_load_fn_symbol_return_false_onerr( g_retro.handle, set_audio_sample_batch,
                                               retro_set_audio_sample_batch );

    set_environment( core_environment );
    set_video_refresh( core_video_refresh );
    set_input_poll( core_input_poll );
    set_input_state( core_input_state );
    set_audio_sample( core_audio_sample );
    set_audio_sample_batch( core_audio_sample_batch );

    g_retro.retro_init();
    g_retro.initialized = true;

    retro_system_info info;
    g_retro.retro_get_system_info( &info );

    godot::UtilityFunctions::print( "Core info:" );
    godot::UtilityFunctions::print( "  library_name: ", info.library_name );
    godot::UtilityFunctions::print( "  library_version: ", info.library_version );
    godot::UtilityFunctions::print( "  valid_extensions: ", info.valid_extensions );
    godot::UtilityFunctions::print( "  need_fullpath: ", info.need_fullpath );
    godot::UtilityFunctions::print( "  block_extract: ", info.block_extract );

    if ( !core_load_game( NULL ) )
    {
        return false;
    }

    g_retro.retro_run();

    return true;
}

void LibRetroHost::unload_core()
{
    godot::UtilityFunctions::print( "Unloading core" );
    if ( g_retro.initialized )
    {
        g_retro.retro_deinit();
        g_retro.initialized = false;
    }
    if ( g_retro.handle != NULL )
    {
        FreeLibrary( g_retro.handle );
        g_retro.handle = NULL;
    }
}

void LibRetroHost::run()
{
    if(!g_retro.initialized) {
      godot::UtilityFunctions::printerr( "Core not initialized" );
      return;
    }
    g_retro.retro_run();
}

godot::Ref<godot::Image> LibRetroHost::get_frame_buffer() {
    return frame_buffer;
}

void LibRetroHost::_bind_methods()
{
    godot::ClassDB::bind_static_method( "LibRetroHost", godot::D_METHOD( "load_core", "path" ),
                                        &LibRetroHost::load_core );

    godot::ClassDB::bind_static_method( "LibRetroHost", godot::D_METHOD( "unload_core" ),
                                        &LibRetroHost::unload_core );

    godot::ClassDB::bind_static_method( "LibRetroHost", godot::D_METHOD( "run" ), &LibRetroHost::run );

    godot::ClassDB::bind_static_method( "LibRetroHost", godot::D_METHOD( "get_frame_buffer" ),
                                        &LibRetroHost::get_frame_buffer );
}