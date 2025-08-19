#include <windows.h>
#include <stdint.h>
#include <Xinput.h>
#include <xaudio2.h>

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

struct Win32OffScreenBuffer
{
    BITMAPINFO info;
    void *memory;
    int width;
    int height;
    int pitch;
};

struct Win32WindowDimension
{
    int width;
    int height;
};

// TODO(Gary): temporarily global for now
static bool isRunning;
static Win32OffScreenBuffer globalBackBuffer;

// NOTE(gary): XInputGetState
#define X_INPUT_GET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_STATE *pState)
typedef X_INPUT_GET_STATE(x_input_get_state);
X_INPUT_GET_STATE(XInputGetStateStub)
{
    // NOTE(gary): in Windows, 0 is ERROR_SUCCESS = success so have to provide some error code in order to make this function failed.
    return ERROR_NOT_CONNECTED;
}
static x_input_get_state *XInputGetState_ = XInputGetStateStub;
#define XInputGetState XInputGetState_

// NOTE(gary): XInputSetState
#define X_INPUT_SET_STATE(name) DWORD WINAPI name(DWORD dwUserIndex, XINPUT_VIBRATION *pVibration)
typedef X_INPUT_SET_STATE(x_input_set_state);
X_INPUT_SET_STATE(XInputSetStateStub)
{
    // NOTE(gary): in Windows, 0 is ERROR_SUCCESS = success so have to provide some error code in order to make this function failed.
    return ERROR_NOT_CONNECTED;
}
static x_input_set_state *XInputSetState_ = XInputSetStateStub;
#define XInputSetState XInputSetState_

// NOTE(gary): XAudio2Create
#define X_AUDIO2_CREATE(name) HRESULT name(IXAudio2 **ppXAudio2, UINT32 Flags, XAUDIO2_PROCESSOR XAudio2Processor)
typedef X_AUDIO2_CREATE(x_audio2_create);
X_AUDIO2_CREATE(XAudio2CreateStub)
{
    return XAUDIO2_E_XAPO_CREATION_FAILED;
}

static void Win32LoadXInputDLL()
{
    HMODULE xInputLibrary = LoadLibraryA("xinput1_4.dll");
    if (!xInputLibrary)
    {
        // TODO(gary): Diagnostic

        xInputLibrary = LoadLibraryA("xinput1_3.dll");
    }

    if (xInputLibrary)
    {
        XInputGetState = (x_input_get_state *)GetProcAddress(xInputLibrary, "XInputGetState");
        XInputSetState = (x_input_set_state *)GetProcAddress(xInputLibrary, "XInputSetState");
    }
    else
    {
        // TODO(gary): Diagnostic
    }
}

static void Win32InitXAudio2(int32 samplesPerSecond)
{
    HMODULE xAudioLibrary = LoadLibraryA("xaudio2_9.dll");
    if (!xAudioLibrary)
    {
        // TODO(gary): Diagnostic

        xAudioLibrary = LoadLibraryA("xaudio2_8.dll");
    }

    if (xAudioLibrary)
    {
        x_audio2_create *XAudio2Create = (x_audio2_create *)GetProcAddress(xAudioLibrary, "XAudio2Create");

        IXAudio2 *audio = {};
        if (SUCCEEDED(XAudio2Create(&audio, 0, XAUDIO2_DEFAULT_PROCESSOR)))
        {
            IXAudio2MasteringVoice *masteringVoice = {};
            if (SUCCEEDED(audio->CreateMasteringVoice(&masteringVoice)))
            {
                IXAudio2SourceVoice *sourceVoice = {};
                WAVEFORMATEX waveFormat = {};
                waveFormat.wFormatTag = WAVE_FORMAT_PCM;
                waveFormat.nChannels = 2;
                waveFormat.nSamplesPerSec = samplesPerSecond;
                waveFormat.wBitsPerSample = 16;
                // NOTE(gary): (nChannels * wBitsPerSample) / 8
                waveFormat.nBlockAlign = (waveFormat.nChannels * waveFormat.wBitsPerSample) / 8;
                waveFormat.nAvgBytesPerSec = waveFormat.nSamplesPerSec * waveFormat.nBlockAlign;

                if (SUCCEEDED(audio->CreateSourceVoice(&sourceVoice, &waveFormat)))
                {
                    // TODO(gary): not sure how to fill out buffer.AudioBytes
                    XAUDIO2_BUFFER buffer = {};
                    buffer.Flags = XAUDIO2_END_OF_STREAM;
                    // buffer.AudioBytes = ;
                    // buffer.pAudioData = ;
                    // buffer.PlayBegin = ;
                    // buffer.PlayLength = ;
                    // buffer.LoopBegin = ;
                    // buffer.LoopLength = ;
                    // buffer.LoopCount = ;
                    // buffer.pContext = ;
                }
                else
                {
                    // TODO(gary): Diagnostic
                }
            }
            else
            {
                // TODO(gary): Diagnostic
            }
        }
    }
    else
    {
        // TODO(gary): Diagnostic
    }
}

static Win32WindowDimension Win32GetWindowDimension(HWND window)
{
    Win32WindowDimension result;

    RECT clientRect;
    GetClientRect(window, &clientRect);

    result.width = clientRect.right - clientRect.left;
    result.height = clientRect.bottom - clientRect.top;

    return result;
}

static void RenderWeirdGradient(Win32OffScreenBuffer *buffer, int blueOffset, int greenOffset)
{
    /*
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

// DIB = Device-Independent Section (https://learn.microsoft.com/en-us/windows/win32/gdi/device-independent-bitmaps)
static void Win32ResizeDIBSection(Win32OffScreenBuffer *buffer, int width, int height)
{
    if (buffer->memory)
    {
        VirtualFree(buffer->memory, 0, MEM_RELEASE);
    }

    buffer->width = width;
    buffer->height = height;

    int bytesPerPixel = 4;

    buffer->info.bmiHeader.biSize = sizeof(buffer->info.bmiHeader);
    buffer->info.bmiHeader.biWidth = buffer->width;
    // If positive, is bottom-up. If negative, is top-down
    buffer->info.bmiHeader.biHeight = -buffer->height;
    buffer->info.bmiHeader.biPlanes = 1;
    buffer->info.bmiHeader.biBitCount = 32;
    buffer->info.bmiHeader.biCompression = BI_RGB;

    int bitmapMemorySize = (buffer->width * buffer->height) * bytesPerPixel;
    buffer->memory = VirtualAlloc(0, bitmapMemorySize, MEM_COMMIT, PAGE_READWRITE);
    buffer->pitch = buffer->width * bytesPerPixel;
}

static void Win32DisplayBufferWindow(Win32OffScreenBuffer *buffer, HDC deviceContext, int windowWidth, int windowHeight)
{
    // TODO(gary): Aspect ratio correction
    StretchDIBits(
        deviceContext,
        // x, y, width, height,
        // x, y, width, height,
        0, 0, windowWidth, windowHeight,
        0, 0, buffer->width, buffer->height,
        buffer->memory, &buffer->info,
        DIB_RGB_COLORS, SRCCOPY);
}

LRESULT CALLBACK Win32MainWindowCallback(HWND window, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    LRESULT result = 0;

    switch (uMsg)
    {
    case WM_ACTIVATEAPP:
    {
        OutputDebugStringA("WM_ACTIVATEAPP\n");
    }
    break;

    case WM_CLOSE:
    {
        // TODO(gary): handle this as message to the user?
        isRunning = false;
    }
    break;

    case WM_DESTROY:
    {
        // TODO(gary): handle this as error -> recreate window?
        isRunning = false;
    }
    break;

    case WM_SYSKEYUP:
    case WM_SYSKEYDOWN:
    case WM_KEYUP:
    case WM_KEYDOWN:
    {
        // NOTE(gary): https://learn.microsoft.com/en-us/windows/win32/inputdev/keyboard-input-notifications
        uint32 vkCode = wParam;
        bool wasDown = (lParam & (1 << 30)) != 0;
        bool isDown = (lParam & (1 << 31)) == 0;

        if (wasDown != isDown)
        {
            if (vkCode == 'W')
            {
                //
            }
            else if (vkCode == 'A')
            {
                //
            }
            else if (vkCode == 'S')
            {
                //
            }
            else if (vkCode == 'D')
            {
                //
            }
            else if (vkCode == VK_UP)
            {
                //
            }
            else if (vkCode == VK_DOWN)
            {
                //
            }
            else if (vkCode == VK_LEFT)
            {
                //
            }
            else if (vkCode == VK_RIGHT)
            {
                //
            }
            else if (vkCode == VK_ESCAPE)
            {
                //
            }
            else if (vkCode == VK_SPACE)
            {
                OutputDebugStringA("SPACE: ");
                if (isDown)
                {
                    OutputDebugStringA("isDown ");
                }
                if (wasDown)
                {
                    OutputDebugStringA("wasDown");
                }
                OutputDebugStringA("\n");
            }

            // NOTE(gary): WM_SYSKEYDOWN & WM_SYSKEYUP message
            bool32 altKeyWasDown = (lParam & (1 << 29));
            if (vkCode == VK_F4 && altKeyWasDown)
            {
                isRunning = false;
            }
        }
    }
    break;

    case WM_PAINT:
    {
        PAINTSTRUCT paint;
        HDC deviceContext = BeginPaint(window, &paint);
        Win32WindowDimension dimension = Win32GetWindowDimension(window);

        Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);

        EndPaint(window, &paint);
    }
    break;

    default:
    {
        result = DefWindowProcA(window, uMsg, wParam, lParam);
    }
    break;
    }

    return result;
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, PSTR cmdline, int cmdshow)
{
    Win32LoadXInputDLL();

    WNDCLASSA windowClass = {};

    Win32ResizeDIBSection(&globalBackBuffer, 1280, 720);

    windowClass.style = CS_HREDRAW | CS_VREDRAW | CS_OWNDC;
    windowClass.lpfnWndProc = Win32MainWindowCallback;
    windowClass.hInstance = hInstance;
    // windowClass.hIcon = ;
    windowClass.lpszClassName = "HandmadeHeroWindowClass";

    if (RegisterClassA(&windowClass))
    {
        HWND window = CreateWindowExA(
            0, windowClass.lpszClassName, "Handmade Hero", WS_OVERLAPPEDWINDOW | WS_VISIBLE,
            CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
            0, 0, hInstance, 0);

        if (window)
        {
            Win32InitXAudio2(48000);

            HDC deviceContext = GetDC(window);

            int xOffset = 0;
            int yOffset = 0;

            isRunning = true;

            while (isRunning)
            {

                MSG message;
                while (PeekMessageA(&message, 0, 0, 0, PM_REMOVE))
                {
                    if (message.message == WM_QUIT)
                    {
                        isRunning = false;
                    }

                    TranslateMessage(&message);
                    DispatchMessageA(&message);
                }

                // NOTE(gary): Xinput controller index
                for (DWORD i = 0; i < XUSER_MAX_COUNT; ++i)
                {
                    XINPUT_STATE controllerState;
                    if (XInputGetState(i, &controllerState) == ERROR_SUCCESS)
                    {
                        // Connected
                        XINPUT_GAMEPAD *gamepad = &controllerState.Gamepad;

                        bool up = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_UP);
                        bool down = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_DOWN);
                        bool left = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_LEFT);
                        bool right = (gamepad->wButtons & XINPUT_GAMEPAD_DPAD_RIGHT);
                        bool leftShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_LEFT_SHOULDER);
                        bool rightShoulder = (gamepad->wButtons & XINPUT_GAMEPAD_RIGHT_SHOULDER);
                        bool start = (gamepad->wButtons & XINPUT_GAMEPAD_START);
                        bool back = (gamepad->wButtons & XINPUT_GAMEPAD_BACK);
                        bool AButton = (gamepad->wButtons & XINPUT_GAMEPAD_A);
                        bool BButton = (gamepad->wButtons & XINPUT_GAMEPAD_B);
                        bool XButton = (gamepad->wButtons & XINPUT_GAMEPAD_X);
                        bool YButton = (gamepad->wButtons & XINPUT_GAMEPAD_Y);

                        int16 leftStickX = gamepad->sThumbLX;
                        int16 leftStickY = gamepad->sThumbLY;

                        xOffset += leftStickX >> 12;
                        yOffset -= leftStickY >> 12;
                    }
                    else
                    {
                        // TODO(gary): Diagnostic
                    }
                }

                // XINPUT_VIBRATION vibration;
                // vibration.wLeftMotorSpeed = 60000;
                // vibration.wRightMotorSpeed = 60000;
                // XInputSetState(0, &vibration);

                RenderWeirdGradient(&globalBackBuffer, xOffset, yOffset);

                Win32WindowDimension dimension = Win32GetWindowDimension(window);
                Win32DisplayBufferWindow(&globalBackBuffer, deviceContext, dimension.width, dimension.height);
                ReleaseDC(window, deviceContext);

                ++xOffset;
            }
        }
        else
        {
            // TODO(gary): logging
        }
    }
    else
    {
        // TODO(gary): logging
    }

    return 0;
}