#pragma once
#include "input_events.hpp"
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
#include <string>
#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#endif
namespace keycast {
class WindowsInputPoller {
    mutable bool has_cursor_ = false;
#ifdef _WIN32
    mutable POINT previous_cursor_{};
#endif
    mutable float motion_hold_ = 0.f;
#ifdef _WIN32
    inline static std::atomic<bool> wheel_event_{false};
    std::thread wheel_thread_;
    std::atomic<int> wheel_mode_{0}; // 0 = Raw Input, 1 = low-level hook
    std::atomic<DWORD> wheel_thread_id_{0};
    static LRESULT CALLBACK wheel_callback(int code, WPARAM message, LPARAM data) {
        if (code >= 0 && (message == WM_MOUSEWHEEL || message == WM_MOUSEHWHEEL))
            wheel_event_.store(true, std::memory_order_relaxed);
        return CallNextHookEx(nullptr, code, message, data);
    }
    static LRESULT CALLBACK raw_proc(HWND hwnd, UINT message, WPARAM wparam, LPARAM lparam) {
        if (message == WM_INPUT) {
            RAWINPUT input{};
            UINT size = sizeof(input);
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT, &input,
                                &size, sizeof(RAWINPUTHEADER)) != UINT(-1) &&
                input.header.dwType == RIM_TYPEMOUSE &&
                (input.data.mouse.usButtonFlags & (RI_MOUSE_WHEEL | RI_MOUSE_HWHEEL)))
                wheel_event_.store(true, std::memory_order_relaxed);
        }
        return DefWindowProcW(hwnd, message, wparam, lparam);
    }
    void run_wheel_listener() {
        MSG msg{};
        PeekMessageW(&msg, nullptr, WM_USER, WM_USER, PM_NOREMOVE);
        const HINSTANCE instance = GetModuleHandleW(nullptr);
        const std::wstring class_name = L"KeycastWheel_" + std::to_wstring(GetCurrentThreadId());
        WNDCLASSW cls{};
        cls.lpfnWndProc = raw_proc;
        cls.hInstance = instance;
        cls.lpszClassName = class_name.c_str();
        RegisterClassW(&cls);
        HWND window = CreateWindowExW(0, class_name.c_str(), L"", WS_OVERLAPPED,
                                      0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
        HHOOK hook = nullptr;
        bool raw_registered = false;
        if (window && wheel_mode_.load() == 0) {
            RAWINPUTDEVICE device{};
            device.usUsagePage = 1;
            device.usUsage = 2;
            device.dwFlags = RIDEV_INPUTSINK;
            device.hwndTarget = window;
            raw_registered = RegisterRawInputDevices(&device, 1, sizeof(device)) != FALSE;
        }
        if (wheel_mode_.load() == 1)
            hook = SetWindowsHookExW(WH_MOUSE_LL, wheel_callback, instance, 0);
        wheel_thread_id_.store(GetCurrentThreadId(), std::memory_order_release);
        while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
            TranslateMessage(&msg);
            DispatchMessageW(&msg);
        }
        if (hook) UnhookWindowsHookEx(hook);
        if (raw_registered) {
            RAWINPUTDEVICE device{};
            device.usUsagePage = 1;
            device.usUsage = 2;
            device.dwFlags = RIDEV_REMOVE;
            RegisterRawInputDevices(&device, 1, sizeof(device));
        }
        if (window) DestroyWindow(window);
        UnregisterClassW(class_name.c_str(), instance);
    }
    void stop_wheel_listener() {
        if (!wheel_thread_.joinable()) return;
        while (wheel_thread_id_.load(std::memory_order_acquire) == 0)
            std::this_thread::yield();
        PostThreadMessageW(wheel_thread_id_.load(std::memory_order_acquire), WM_QUIT, 0, 0);
        wheel_thread_.join();
        wheel_thread_id_.store(0, std::memory_order_release);
    }
    void start_wheel_listener() {
        wheel_event_.store(false, std::memory_order_relaxed);
        wheel_thread_ = std::thread([this] { run_wheel_listener(); });
    }
#endif
    mutable float wheel_hold_ = 0.f;
public:
    WindowsInputPoller() {
#ifdef _WIN32
        start_wheel_listener();
#endif
    }
    ~WindowsInputPoller() {
#ifdef _WIN32
        stop_wheel_listener();
#endif
    }
    void set_wheel_mode(int mode) {
#ifdef _WIN32
        mode = mode == 1 ? 1 : 0;
        if (wheel_mode_.load() == mode) return;
        stop_wheel_listener();
        wheel_mode_.store(mode);
        start_wheel_listener();
#else
        (void)mode;
#endif
    }
    WindowsInputPoller(const WindowsInputPoller &) = delete;
    WindowsInputPoller &operator=(const WindowsInputPoller &) = delete;
    struct MouseMotion {
        float x = 0.f;
        float y = 0.f;
        bool moving = false;
    };
    MouseMotion mouse_motion(float dt) const {
        float dx = 0.f, dy = 0.f;
#ifdef _WIN32
        POINT point{};
        if (GetCursorPos(&point)) {
            if (has_cursor_) {
                dx = static_cast<float>(point.x - previous_cursor_.x);
                dy = static_cast<float>(point.y - previous_cursor_.y);
                if (dx * dx + dy * dy >= 4.f) motion_hold_ = 0.12f;
            }
            previous_cursor_ = point;
            has_cursor_ = true;
        } else {
            has_cursor_ = false;
        }
#endif
        const bool moving = motion_hold_ > 0.f;
        motion_hold_ = std::max(0.f, motion_hold_ - std::max(0.f, dt));
        // Cursor velocity is bounded so a screen-edge jump cannot fling the hand.
        return {std::clamp(dx / 35.f, -1.f, 1.f),
                std::clamp(dy / 35.f, -1.f, 1.f), moving};
    }
    bool mouse_moving(float dt) const { return mouse_motion(dt).moving; }
    void poll(InputState &state, float dt = 0.f) const {
#ifdef _WIN32
        // Poll the full Windows virtual-key range for configured PNG overlays.
        for (uint32_t key = 0x08u; key <= 0xFEu; ++key)
            state.apply({InputKind::Key, key, (GetAsyncKeyState(static_cast<int>(key)) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 1, (GetAsyncKeyState(VK_LBUTTON) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 2, (GetAsyncKeyState(VK_RBUTTON) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 3, (GetAsyncKeyState(VK_MBUTTON) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 4, (GetAsyncKeyState(VK_XBUTTON1) & 0x8000) ? 1.0f : 0.0f});
        state.apply({InputKind::MouseButton, 5, (GetAsyncKeyState(VK_XBUTTON2) & 0x8000) ? 1.0f : 0.0f});
        if (wheel_event_.exchange(false, std::memory_order_relaxed))
            wheel_hold_ = 0.12f;
        const bool wheel_active = wheel_hold_ > 0.f;
        wheel_hold_ = std::max(0.f, wheel_hold_ - std::max(0.f, dt));
        state.apply({InputKind::MouseButton, 6, wheel_active ? 1.0f : 0.0f});
#else
        (void)state;
        (void)dt;
#endif
    }
};
}
