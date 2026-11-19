#include <chrono>
#include <cstdint>
#include <cstdio>

#include <windows.h>

#pragma comment(lib, "user32.lib")

namespace {

constexpr double kMouseDpi = 18000.0;
constexpr UINT_PTR kMouseTimerId = 1;
constexpr UINT kSampleIntervalMs = 16;

struct MouseVelocity {
    double xMetersPerSecond;
    double yMetersPerSecond;
};

int64_t pendingX = 0;
int64_t pendingY = 0;
auto previousSampleTime = std::chrono::steady_clock::now();

MouseVelocity TrackMouse() {
    const auto now = std::chrono::steady_clock::now();
    const std::chrono::duration<double> elapsed = now - previousSampleTime;
    previousSampleTime = now;

    const double metersPerCount = 0.0254 / kMouseDpi;
    const MouseVelocity velocity = {
        static_cast<double>(pendingX) * metersPerCount / elapsed.count(),
        static_cast<double>(pendingY) * metersPerCount / elapsed.count()
    };
    pendingX = 0;
    pendingY = 0;
    return velocity;
}

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
            pendingX += deltaX;
            pendingY += deltaY;
        }

        return DefWindowProcW(window, message, wParam, lParam);
    }

    if (message == WM_TIMER && wParam == kMouseTimerId) {
        const MouseVelocity velocity = TrackMouse();
        std::printf("velocity x=%.6f m/s  y=%.6f m/s\n",
                    velocity.xMetersPerSecond,
                    velocity.yMetersPerSecond);
        std::fflush(stdout);
        return 0;
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

    if (!SetTimer(receiver, kMouseTimerId, kSampleIntervalMs, nullptr)) {
        std::fprintf(stderr, "SetTimer failed: %lu\n",
                     static_cast<unsigned long>(GetLastError()));
        DestroyWindow(receiver);
        return 1;
    }

    std::puts("Tracking raw mouse movement system-wide. Press Ctrl+C to stop.");
    std::printf("Velocity is sampled every %u ms using a %.0f dpi mouse.\n",
                kSampleIntervalMs,
                kMouseDpi);
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
