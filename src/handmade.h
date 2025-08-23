#pragma once

struct GameOffscreenBuffer
{
    void *memory;
    int width;
    int height;
    int pitch;
};

static void GameRenderAndUpdate(GameOffscreenBuffer *buffer, int blueOffset, int greenOffset);