#include "handmade.h"

static void GameOutputSound(GameSoundOutputBuffer *soundBuffer, int toneHz)
{
    int16 toneVolume = 3000;

    int16 *samplesOut = soundBuffer->samples;
    for (int i = 0; i < soundBuffer->sampleCount / 2; ++i)
    {
        float t = (float)i / soundBuffer->samplesPerSecond;
        float sineValue = sinf(2.0f * PI * toneHz * t);
        int16 sampleValue = (int16)(sineValue * toneVolume);
        *samplesOut++ = sampleValue;
        *samplesOut++ = sampleValue;
    }
}

static void RenderWeirdGradient(GameOffscreenBuffer *buffer, int blueOffset, int greenOffset)
{
    /*
    NOTE(gary):
                                WIDTH -------->
                                    0                                                  WIDTH * BytesPerPixel
        buffer.memory           0   BB GG RR xx   BB GG RR xx   BB GG RR xx   ...
        buffer.memory + pitch   1   BB GG RR xx   BB GG RR xx
                                ..
    */

    uint8 *row = (uint8 *)buffer->memory;

    for (int y = 0; y < buffer->height; ++y)
    {
        uint32 *pixel = (uint32 *)row;
        // uint8 *pixelOneByte = (uint8 *)row;
        for (int x = 0; x < buffer->width; ++x)
        {
            /*
            NOTE(gary):
            Pixel in memory -> RR GG BB xx
            because of little endian architecture, it became
            Pixel in memory -> BB GG RR xx
            */
            // *pixelOneByte = (uint8)(x + blueOffset);
            // ++pixelOneByte;

            // *pixelOneByte = (uint8)(y + greenOffset);
            // ++pixelOneByte;

            // *pixelOneByte = 0;
            // ++pixelOneByte;

            // *pixelOneByte = 0;
            // ++pixelOneByte;

            uint8 blue = x + blueOffset;
            uint8 green = y + greenOffset;

            /*
            Pixel (32-bit)
            memory:      BB GG RR xx
            register:    xx RR GG BB
            */
            *pixel++ = ((green << 8) | blue);
        }

        row += buffer->pitch;
    }
}

static void GameRenderAndUpdate(GameOffscreenBuffer *buffer, GameSoundOutputBuffer *soundBuffer,
                                int blueOffset, int greenOffset, int toneHz)
{
    GameOutputSound(soundBuffer, toneHz);
    RenderWeirdGradient(buffer, blueOffset, greenOffset);
}