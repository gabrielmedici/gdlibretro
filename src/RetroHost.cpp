#include "RetroHost.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include <godot_cpp/classes/file_access.hpp>
#include <godot_cpp/classes/os.hpp>
#include <godot_cpp/classes/project_settings.hpp>

// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
// Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsStr()
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

RetroHost::RetroHost()
{
    godot::UtilityFunctions::print( "[RetroHost] Constructor" );
    singleton = this;
}

RetroHost::~RetroHost()
{
    godot::UtilityFunctions::print( "[RetroHost] Destructor" );
    this->unload_core();
}

RetroHost *RetroHost::singleton = nullptr;

RetroHost *RetroHost::get_singleton()
{
    return singleton;
}

#define load_symbol_return_false_on_err( handle, dest, sym )                                       \
    godot::UtilityFunctions::print( "[RetroHost] Loading core symbol \"", #sym, "\"" );            \
    dest = (decltype( dest ))GetProcAddress( handle, #sym );                                       \
    if ( dest == NULL )                                                                            \
    {                                                                                              \
        godot::UtilityFunctions::printerr( "[RetroHost] Could not load symbol \"", #sym,           \
                                           "\": ", GetLastErrorAsStr().c_str() );                  \
        return false;                                                                              \
    }

#define def_and_load_fn_symbol_return_false_on_err( handle, dest, sym )                            \
    godot::UtilityFunctions::print( "[RetroHost] Loading core symbol \"", #sym, "\"" );            \
    decltype( sym ) *dest = reinterpret_cast<decltype( sym ) *>( GetProcAddress( handle, #sym ) ); \
    if ( dest == nullptr )                                                                         \
    {                                                                                              \
        godot::UtilityFunctions::printerr( "[RetroHost] Could not load symbol \"", #sym,           \
                                           "\": ", GetLastErrorAsStr().c_str() );                  \
        return false;                                                                              \
    }

bool RetroHost::load_core( godot::String name )
{
    this->unload_core();
    godot::UtilityFunctions::print( "[RetroHost] Loading core \"", name, "\"" );

    godot::String dll_path;

    if ( godot::OS::get_singleton()->has_feature( "editor" ) )
    {
        // MAYBE TODO: Add a setting to change the core base path in the editor
        // the current working directory in the editor is the godot executable itself, but we want
        // to load the cores from the project directory
        this->cwd = godot::ProjectSettings::get_singleton()->globalize_path( "res://" );
        dll_path = cwd + "libretro-cores/" + name + ".dll";
    }
    else
    {
        // Windows can handle loading libraries by name alone if we're in a exported build, so we
        // don't need to do anything special here
        this->cwd = godot::OS::get_singleton()->get_executable_path().get_base_dir();
        dll_path = name;
    }

    this->core.handle = LoadLibrary( dll_path.utf8().get_data() );

    if ( this->core.handle == NULL )
    {
        godot::UtilityFunctions::print( "[RetroHost] Failed to load core \"", dll_path, "\"" );
        return false;
    }

    load_symbol_return_false_on_err( this->core.handle, this->core.retro_init, retro_init );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_deinit, retro_deinit );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_api_version,
                                     retro_api_version );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_get_system_info,
                                     retro_get_system_info );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_get_system_av_info,
                                     retro_get_system_av_info );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_set_controller_port_device,
                                     retro_set_controller_port_device );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_reset, retro_reset );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_run, retro_run );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_load_game,
                                     retro_load_game );
    load_symbol_return_false_on_err( this->core.handle, this->core.retro_unload_game,
                                     retro_unload_game );

    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_environment,
                                                retro_set_environment );
    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_video_refresh,
                                                retro_set_video_refresh );
    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_input_poll,
                                                retro_set_input_poll );
    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_input_state,
                                                retro_set_input_state );
    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_audio_sample,
                                                retro_set_audio_sample );
    def_and_load_fn_symbol_return_false_on_err( this->core.handle, set_audio_sample_batch,
                                                retro_set_audio_sample_batch );

    // May god have mercy on our souls for the crimes we are about to commit
    // Blame c++

    set_environment(
        []( unsigned cmd, void *data ) { return get_singleton()->core_environment( cmd, data ); } );

    set_video_refresh( []( const void *data, unsigned width, unsigned height, size_t pitch ) {
        get_singleton()->core_video_refresh( data, width, height, pitch );
    } );

    set_input_poll( []( void ) { get_singleton()->core_input_poll(); } );

    set_input_state( []( unsigned port, unsigned device, unsigned index, unsigned id ) {
        return get_singleton()->core_input_state( port, device, index, id );
    } );

    set_audio_sample(
        []( int16_t left, int16_t right ) { get_singleton()->core_audio_sample( left, right ); } );

    set_audio_sample_batch( []( const int16_t *data, size_t frames ) {
        return get_singleton()->core_audio_sample_batch( data, frames );
    } );

    this->core.retro_init();

    // End of c++ crimes

    this->core.retro_load_game( NULL );

    struct retro_system_av_info av;
    this->core.retro_get_system_av_info( &av );

    this->core_video_init( &av.geometry );
    this->core_audio_init( av );

    this->core.initialized = true;

    this->core.retro_run();

    return true;
}

void RetroHost::unload_core()
{
    if ( this->core.initialized )
    {
        godot::UtilityFunctions::print( "[RetroHost] Deinitializing core" );
        this->core.retro_deinit();
        this->core.initialized = false;
    }
    if ( this->core.handle != NULL )
    {
        godot::UtilityFunctions::print( "[RetroHost] Freeing core library" );
        FreeLibrary( this->core.handle );
        this->core.handle = NULL;
    }
}

void RetroHost::run()
{
    if ( !this->core.initialized )
    {
        godot::UtilityFunctions::printerr( "[RetroHost] Core not initialized" );
        return;
    }
    this->core.retro_run();
}

void RetroHost::_bind_methods()
{
    godot::ClassDB::bind_method( godot::D_METHOD( "load_core", "name" ), &RetroHost::load_core );
    godot::ClassDB::bind_method( godot::D_METHOD( "unload_core" ), &RetroHost::unload_core );
    godot::ClassDB::bind_method( godot::D_METHOD( "run" ), &RetroHost::run );
}