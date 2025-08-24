#pragma once

struct GameOffscreenBuffer
{
    void *memory;
    int width;
    int height;
    int pitch;
};

struct GameSoundOutputBuffer
{
    int samplesPerSecond;
    int sampleCount;
    int16 *samples;
};

static void GameRenderAndUpdate(GameOffscreenBuffer *buffer, GameSoundOutputBuffer *soundBuffer,
                                int blueOffset, int greenOffset, int toneHz);