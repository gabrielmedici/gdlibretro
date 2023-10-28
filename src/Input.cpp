#include "RetroHost.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void RetroHost::core_input_poll( void )
{
    //godot::UtilityFunctions::print( "Input poll" );
}

int16_t RetroHost::core_input_state( unsigned port, unsigned device, unsigned index, unsigned id )
{

    switch( device ) {
        case RETRO_DEVICE_KEYBOARD:
            //godot::UtilityFunctions::print( "Port ", port, " Device ", device, " Id ", id, " Idx ", index );
            return false;
    }
    return 0;
}