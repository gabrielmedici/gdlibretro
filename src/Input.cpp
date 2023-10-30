#include "RetroHost.hpp"
#include "KeyboardMap.hpp"
#include "godot_cpp/classes/input_event_key.hpp"
#include "godot_cpp/variant/utility_functions.hpp"

void RetroHost::core_input_poll( void )
{
    // godot::UtilityFunctions::print( "Input poll" );
}

int16_t RetroHost::core_input_state( unsigned port, unsigned device, unsigned index, unsigned id )
{

    switch ( device )
    {
        case RETRO_DEVICE_KEYBOARD:
            // godot::UtilityFunctions::print( "Port ", port, " Device ", device, " Id ", id, " Idx
            // ", index );
            return false;
    }
    return 0;
}

void RetroHost::forwarded_input( const godot::Ref<godot::InputEvent> &event )
{
    if ( event->is_class( "InputEventKey" ) )
    {
        auto key_event = godot::Object::cast_to<godot::InputEventKey>( event.ptr() );
        godot::UtilityFunctions::print( "Key event: echo:", key_event->is_echo(),
                                        " key label: ", key_event->get_key_label(),
                                        " key code: ", key_event->get_keycode(),
                                        " physical key w/ mod: ", key_event->get_physical_keycode_with_modifiers(),
                                        " physical key: ", key_event->get_physical_keycode(),
                                        " pressed: ", key_event->is_pressed() );

        // Up the bool value 0x1 to 0xFF so we have enough bits for the & operation
        uint16_t modifiers = (RETROKMOD_ALT & ( key_event->is_alt_pressed() ? 0xFF : 0 )) |
                             (RETROKMOD_CTRL & ( key_event->is_ctrl_pressed() ? 0xFF : 0 )) |
                             (RETROKMOD_META & ( key_event->is_meta_pressed() ? 0xFF : 0 )) |
                             (RETROKMOD_SHIFT & ( key_event->is_shift_pressed() ? 0xFF : 0 ));

        godot::UtilityFunctions::print( "Modifiers: ", modifiers );

        this->core.retro_keyboard_event_callback( key_event->is_pressed(), godotKeyToRetroKey(key_event->get_keycode_with_modifiers()),
                                                  0, modifiers );
    }
}