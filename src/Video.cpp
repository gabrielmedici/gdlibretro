#include "RetroHost.hpp"
#include "godot_cpp/variant/utility_functions.hpp"
#include "godot_cpp/classes/image.hpp"

void RetroHost::core_video_init( const struct retro_game_geometry *geometry )
{
}
void RetroHost::core_video_refresh( const void *data, unsigned width, unsigned height,
                                    size_t pitch )
{
}

bool RetroHost::core_video_set_pixel_format( unsigned format ) {
    switch ( format )
    {
        case RETRO_PIXEL_FORMAT_0RGB1555:
            godot::UtilityFunctions::print( "Pixel format: 0RGB1555" );
            this->pixel_format = godot::Image::Format::FORMAT_RGB565;
            return true;
        case RETRO_PIXEL_FORMAT_XRGB8888:
            godot::UtilityFunctions::print( "Pixel format: XRGB8888" );
            this->pixel_format = godot::Image::Format::FORMAT_RGB8;
            return true;
        case RETRO_PIXEL_FORMAT_RGB565:
            godot::UtilityFunctions::print( "Pixel format: RGB565" );
            this->pixel_format = godot::Image::Format::FORMAT_RGB565;
            return true;
        default:
            return false;
    }

}