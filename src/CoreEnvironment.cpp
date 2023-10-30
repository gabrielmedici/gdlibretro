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

    godot::UtilityFunctions::print( "[RetroHost Loaded CORE][" +
                                    godot::String( levelstr[level - 1] ) + "] " + buffer );
}

bool RetroHost::get_variable( retro_variable *variable )
{
    if ( !this->core_variables[variable->key].IsDefined() )
    {
        godot::UtilityFunctions::printerr( "[RetroHost] Core variable ", variable->key, " not defined" );
        return false;
    }

    auto var_value = core_variables[variable->key].as<std::string>();
    if(var_value.empty()) {
        godot::UtilityFunctions::printerr( "[RetroHost] Core variable ", variable->key, " was empty ", var_value.c_str() );
        return false;
    }

    // var_value goes out of scope after this function returns, so we need to copy it
    const std::string::size_type size = var_value.size();
    char *buffer = new char[size + 1];
    memcpy(buffer, var_value.c_str(), size + 1);

    // No leaks here please thank you
    this->please_free_me_str.push_back(buffer);

    variable->value = buffer;
    return true;
}

std::vector<std::string> split( std::string s, std::string delimiter )
{
    size_t pos_start = 0, pos_end, delim_len = delimiter.length();
    std::string token;
    std::vector<std::string> res;

    while ( ( pos_end = s.find( delimiter, pos_start ) ) != std::string::npos )
    {
        token = s.substr( pos_start, pos_end - pos_start );
        pos_start = pos_end + delim_len;
        res.push_back( token );
    }

    res.push_back( s.substr( pos_start ) );
    return res;
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
            auto var = (retro_variable *)data;
            auto result = this->get_variable( var );
            // if ( result )
            // {
            //     godot::UtilityFunctions::print( "[RetroHost] Core variable ",
            //                                     var->key, " set to ",
            //                                     var->value );
            // }
            // else
            // {
            //     godot::UtilityFunctions::print( "[RetroHost] Core variable set request ",
            //                                     var->key, " not handled" );
            // }
            return result;
        }
        break;

        case RETRO_ENVIRONMENT_SET_VARIABLES:
        {
            auto variables = (const struct retro_variable *)data;
            while ( variables->key )
            {
                if(!this->core_variables[variables->key].IsDefined()) {
                    std::string value = variables->value;
                    // Get the possible values from the value string and trim the leading space
                    auto possible_values_str = split(value, ";")[1].erase(0, 1);
                    // Split the possible values into a vector
                    auto possible_values = split(possible_values_str, "|");
                    // Set the value to the first possible value
                    this->core_variables[variables->key] = possible_values[0];

                    godot::UtilityFunctions::print( "[RetroHost] Core variable ", variables->key, " was not present in the config file, now set to the first possible value: ", possible_values[0].c_str() );
                }
                variables++;
            }
        }

        case RETRO_ENVIRONMENT_GET_VARIABLE_UPDATE:
        {
            // Just say no variable was updated
            // TODO: Actually do something
            bool *b = (bool *)data;
            *b = false;
        }
        break;

        // TODO: Implement (DOS)
        // case RETRO_ENVIRONMENT_GET_PERF_INTERFACE: {
        //     // struct retro_perf_callback
        //     auto callback = (struct retro_perf_callback *)data;
        // }


        // TODO: Implement (DOS)
        // case RETRO_ENVIRONMENT_SET_DISK_CONTROL_INTERFACE: {
        //     auto callback = (struct retro_disk_control_callback *)data;
        //     godot::UtilityFunctions::print( "[RetroHost] disk control interface set" );
        // }
        // break;

        // TODO: Implement (DOS)
        // case RETRO_ENVIRONMENT_SET_DISK_CONTROL_EXT_INTERFACE: {

        // }

        // TODO: Implement (DOS)
        // case RETRO_ENVIRONMENT_SET_VARIABLE: {

        // }
        // break;

        // TODO: Implement (DOS) and understand wtf achievements are supposed to be
        // case RETRO_ENVIRONMENT_SET_SUPPORT_ACHIEVEMENTS: {

        // }
        // break;

        // TODO: Implement (DOS)
        // case RETRO_ENVIRONMENT_SET_CORE_OPTIONS_UPDATE_DISPLAY_CALLBACK: {

        // }
        // break;

        // TODO: Implement (DOS) not really important
        // case RETRO_ENVIRONMENT_SET_CONTROLLER_INFO: {

        // }

        case RETRO_ENVIRONMENT_GET_CORE_OPTIONS_VERSION: {
            unsigned *version = (unsigned *)data;
            // TODO: Support higher versions
            *version = 0;
            return false;
        }

        case RETRO_ENVIRONMENT_GET_MESSAGE_INTERFACE_VERSION: {
            unsigned *version = (unsigned *)data;
            *version = 0;
            return false;
        }
        break;

        case RETRO_ENVIRONMENT_SET_PIXEL_FORMAT:
        {
            const enum retro_pixel_format *fmt = (enum retro_pixel_format *)data;

            if ( *fmt > RETRO_PIXEL_FORMAT_RGB565 )
            {
                return false;
            }

            godot::UtilityFunctions::print( "[RetroHost] Core setting pixel format" );
            return this->core_video_set_pixel_format( *fmt );
        }
        break;

        case RETRO_ENVIRONMENT_SET_KEYBOARD_CALLBACK: {
            // const struct retro_keyboard_callback
            auto callback = (const struct retro_keyboard_callback *)data;
            this->core.retro_keyboard_event_callback = callback->callback;
            godot::UtilityFunctions::print( "[RetroHost] keyboard callback set" );
        }
        break;

        case RETRO_ENVIRONMENT_GET_INPUT_BITMASKS: {
            if(!data)
                return false;
            // TODO: Support bitmasks
            bool *b = (bool *)data;
            *b = false;
        }
        break;

        case RETRO_ENVIRONMENT_SET_INPUT_DESCRIPTORS: {
            const struct retro_input_descriptor *desc = (const struct retro_input_descriptor *)data;
            while (desc->description) {
                godot::UtilityFunctions::print("[RetroHost] Core input descriptor: ", desc->description);
                desc++;
            }
        }
        break;

        case RETRO_ENVIRONMENT_SET_SUPPORT_NO_GAME: {
            bool *b = (bool *)data;
            if(*b) {
                godot::UtilityFunctions::print("[RetroHost] Core supports no game");
            } else {
                godot::UtilityFunctions::print("[RetroHost] Core does not support no game");
            }
        }

        case RETRO_ENVIRONMENT_GET_THROTTLE_STATE: {
            // retro_throttle_state
            auto throttle_state = (struct retro_throttle_state *)data;
            throttle_state->mode = RETRO_THROTTLE_UNBLOCKED;
            throttle_state->rate = 0;
            return true;
        }
        break;

        case RETRO_ENVIRONMENT_GET_SYSTEM_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_SAVE_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_CONTENT_DIRECTORY:
        case RETRO_ENVIRONMENT_GET_LIBRETRO_PATH:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core requested path" );
            *(const char **)data = this->cwd.trim_suffix( "/" ).utf8().get_data();
            return true;
        }

        case RETRO_ENVIRONMENT_GET_FASTFORWARDING: {
            if(!data)
                return false;
            bool *b = (bool *)data;
            *b = false;
        }
        break;

        case RETRO_ENVIRONMENT_SHUTDOWN:
        {
            // TODO: Actually shut down the core
            godot::UtilityFunctions::print( "[RetroHost] Core shutdown requested" );
            break;
        }

        default:
        {
            godot::UtilityFunctions::print( "[RetroHost] Core environment command " +
                                            godot::String::num( command ) + " not implemented." );
            return false;
        }
    }

    return true;
}