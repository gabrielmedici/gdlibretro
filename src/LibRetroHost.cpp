#include "godot_cpp/core/class_db.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/file_access.hpp"

#include "LibRetroHost.h"
#include "Version.h"
#include "Windows.h"

LibRetroHost::LibRetroHost()
{
    godot::UtilityFunctions::print("LibRetroHost::LibRetroHost()");
}

LibRetroHost::~LibRetroHost()
{
    godot::UtilityFunctions::print("LibRetroHost::~LibRetroHost()");
    unload_core();
}

// https://stackoverflow.com/questions/1387064/how-to-get-the-error-message-from-the-error-code-returned-by-getlasterror
//Returns the last Win32 error, in string format. Returns an empty string if there is no error.
std::string GetLastErrorAsString()
{
    //Get the error message ID, if any.
    DWORD errorMessageID = ::GetLastError();
    if(errorMessageID == 0) {
        return std::string(); //No error message has been recorded
    }
    
    LPSTR messageBuffer = nullptr;

    //Ask Win32 to give us the string version of that message ID.
    //The parameters we pass in, tell Win32 to create the buffer that holds the message for us (because we don't yet know how long the message string will be).
    size_t size = FormatMessageA(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
                                 NULL, errorMessageID, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), (LPSTR)&messageBuffer, 0, NULL);
    
    //Copy the error message into a std::string.
    std::string message(messageBuffer, size);
    
    //Free the Win32's string's buffer.
    LocalFree(messageBuffer);
            
    return message;
}

#define load_symbol_return_false_onerr(handle, dest, sym) \
    godot::UtilityFunctions::print("Loading core symbol \"", #sym, "\""); \
    dest = (decltype(dest))GetProcAddress(handle, #sym); \
    if(dest == NULL) { \
        godot::UtilityFunctions::printerr("Could not load symbol \"", #sym, "\": ", GetLastErrorAsString().c_str()); \
        return false; \
    }

#define load_retro_symbol_return_false_onerr(sym) load_symbol_return_false_onerr(g_retro.handle, g_retro.sym, sym)

bool LibRetroHost::load_core(godot::String path)
{
    unload_core();

    godot::UtilityFunctions::print("Loading core: ", path);
    g_retro.handle = LoadLibrary((const char *)path.to_utf8_buffer().ptr());

    if(g_retro.handle == NULL) {
        godot::UtilityFunctions::printerr("Could not load DLL: ", GetLastErrorAsString().c_str());
        return false;
    }

    load_retro_symbol_return_false_onerr(retro_init);
    load_retro_symbol_return_false_onerr(retro_deinit);
    load_retro_symbol_return_false_onerr(retro_api_version);
    load_retro_symbol_return_false_onerr(retro_get_system_info);
    load_retro_symbol_return_false_onerr(retro_get_system_av_info);
    load_retro_symbol_return_false_onerr(retro_set_controller_port_device);
    load_retro_symbol_return_false_onerr(retro_reset);
    load_retro_symbol_return_false_onerr(retro_run);

    g_retro.initialized = true;

    godot::UtilityFunctions::print("Retro API version: ", g_retro.retro_api_version());

    return true;
}

void LibRetroHost::unload_core()
{
    godot::UtilityFunctions::print("Unloading core");
    if(g_retro.handle != NULL) {
        FreeLibrary(g_retro.handle);
        g_retro.handle = NULL;
    }
}

void LibRetroHost::_bind_methods()
{
    godot::ClassDB::bind_static_method( "LibRetroHost", godot::D_METHOD( "load_core", "path" ),
                                        &LibRetroHost::load_core );
}