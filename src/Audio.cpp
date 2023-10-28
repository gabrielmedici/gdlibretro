#include "RetroHost.hpp"

void RetroHost::core_audio_init( retro_system_av_info av )
{
}

void RetroHost::core_audio_sample( int16_t left, int16_t right )
{
}

size_t RetroHost::core_audio_sample_batch( const int16_t *data, size_t frames )
{
    return 0;
}