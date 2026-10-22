#include <windows.h>

#include <cstdint>
#include <cstdio>

#pragma comment(lib, "user32.lib")

namespace {

int64_t totalX = 0;
int64_t totalY = 0;

LRESULT CALLBACK WindowProc(HWND window, UINT message, WPARAM wParam, LPARAM lParam) {
    if (message == WM_INPUT) {
        RAWINPUT rawInput = {};
        UINT size = sizeof(rawInput);
        const UINT result = GetRawInputData(
            reinterpret_cast<HRAWINPUT>(lParam),
            RID_INPUT,
            &rawInput,
            &size,
            sizeof(RAWINPUTHEADER));

        if (result == static_cast<UINT>(-1)) {
            std::fprintf(stderr, "GetRawInputData failed: %lu\n",
                         static_cast<unsigned long>(GetLastError()));
        } else if (rawInput.header.dwType == RIM_TYPEMOUSE &&
                   !(rawInput.data.mouse.usFlags & MOUSE_MOVE_ABSOLUTE)) {
            const LONG deltaX = rawInput.data.mouse.lLastX;
            const LONG deltaY = rawInput.data.mouse.lLastY;
            totalX += deltaX;
            totalY += deltaY;

            if (deltaX != 0 || deltaY != 0) {
                std::printf("total x=%lld  y=%lld\n",
                            static_cast<long long>(totalX),
                            static_cast<long long>(totalY));
                std::fflush(stdout);
            }
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    if (message == WM_DESTROY) {
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProcW(window, message, wParam, lParam);
}

}

int main() {
    const HINSTANCE instance = GetModuleHandleW(nullptr);
    WNDCLASSEXW windowClass = {};
    windowClass.cbSize = sizeof(windowClass);
    windowClass.hInstance = instance;
    windowClass.lpfnWndProc = WindowProc;
    windowClass.lpszClassName = L"MouseRawInputReceiver";

    if (!RegisterClassExW(&windowClass)) {
        std::fprintf(stderr, "RegisterClassExW failed: %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        return 1;
    }

    HWND receiver = CreateWindowExW(
        0,
        windowClass.lpszClassName,
        L"",
        0,
        0,
        0,
        0,
        0,
        HWND_MESSAGE,
        nullptr,
        instance,
        nullptr);

    if (!receiver) {
        std::fprintf(stderr, "CreateWindowExW failed: %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        return 1;
    }

    RAWINPUTDEVICE mouseDevice = {};
    mouseDevice.usUsagePage = 0x01;
    mouseDevice.usUsage = 0x02;
    mouseDevice.dwFlags = RIDEV_INPUTSINK;
    mouseDevice.hwndTarget = receiver;
    if (!RegisterRawInputDevices(&mouseDevice, 1, sizeof(mouseDevice))) {
        std::fprintf(stderr, "RegisterRawInputDevices failed: %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        DestroyWindow(receiver);
        return 1;
    }

    std::puts("Tracking raw mouse movement system-wide. Press Ctrl+C to stop.");
    std::puts("Totals are relative mouse movement counts since the program started.");
    std::fflush(stdout);

    MSG message = {};
    BOOL result;
    while ((result = GetMessageW(&message, nullptr, 0, 0)) > 0) {
        TranslateMessage(&message);
        DispatchMessageW(&message);
    }

    if (result == -1) {
        std::fprintf(stderr, "GetMessageW failed: %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        DestroyWindow(receiver);
        return 1;
    }

    return static_cast<int>(message.wParam);
}
