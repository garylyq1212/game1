#include "handmade.h"

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

static void GameRenderAndUpdate(GameOffscreenBuffer *buffer, int blueOffset, int greenOffset)
{
    RenderWeirdGradient(buffer, blueOffset, greenOffset);
}