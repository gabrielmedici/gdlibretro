#include "RetroHost.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void core_log( enum retro_log_level level, const char *fmt, ... )
{
    char buffer[4096] = { 0 };
    static const char *levelstr[] = { "DEBUG", "INFO", "WARN", "ERROR" };
    va_list va;

    va_start( va, fmt );
    vsnprintf( buffer, sizeof( buffer ), fmt, va );
    va_end( va );

    godot::UtilityFunctions::print( "[RetroHost Loaded CORE][" + godot::String( levelstr[level - 1] ) + "] " + buffer );
}

bool get_variable(retro_variable *variable) {
    godot::String key = variable->key;

    if(key == "vice_sound_sample_rate") {
        variable->value = "48000";
        return true;
    } else if (key == "vice_keyboard_keymap") {
        variable->value = "positional";
        return true;
    }

    return false;
}

bool RetroHost::core_environment( unsigned command, void *data )
{
    switch ( command )
    {
        case RETRO_ENVIRONMENT_GET_LOG_INTERFACE:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core log interface set." );
            struct retro_log_callback *cb = (struct retro_log_callback *)data;
            cb->log = core_log;
        }
        break;

        case RETRO_ENVIRONMENT_GET_CAN_DUPE:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core can dupe set." );
            bool *b = (bool *)data;
            *b = true;
        }
        break;

        case RETRO_ENVIRONMENT_GET_VARIABLE:
        {
            auto result = get_variable( (retro_variable *)data );
            if(result) {
                godot::UtilityFunctions::print( "[RetroHost] Core variable ", ((retro_variable *)data)->key, " set to ", ((retro_variable *)data)->value );
            } else {
                godot::UtilityFunctions::print( "[RetroHost] Core variable set request ", ((retro_variable *)data)->key, " not handled" );
            }
            return result;
        }
        break;

        case RETRO_ENVIRONMENT_SET_VARIABLES:
        {
            auto variables = (const struct retro_variable *)data;
            while ( variables->key )
            {
                godot::UtilityFunctions::print( "[RetroHost] Core variable preview ", variables->key);
                variables++;
            }
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        {
            // Just say no variable was updated
            bool *b = (bool *)data;
            *b = false;
        }
        break;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        {
            const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

            if (*fmt > RETRO_PIXEL_FORMAT_RGB565)
                return false;

            godot::UtilityFunctions::print( "[RetroHost] Core setting pixel format");
            return this->core_video_set_pixel_format( *fmt );
        }
        break;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core requested path");
            *(const char **)data = this->cwd.utf8().get_data();
            return true;
        }

        case RETRO_ENVIRONMENT_SHUTDOWN:
        {
            // TODO: Actually shut down the core
            godot::UtilityFunctions::print( "[RetroHost] Core shutdown requested");
            break;
        }

        default:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core environment command " + godot::String::num( command ) + " not implemented." );
            return false;
        }
    }

    return true;
}