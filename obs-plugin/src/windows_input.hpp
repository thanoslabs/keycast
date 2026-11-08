#pragma once
#include "input_events.hpp"
#include <algorithm>
#include <cmath>
#include <atomic>
#include <thread>
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
    std::atomic<DWORD> wheel_thread_id_{0};
    static LRESULT CALLBACK raw_window_proc(HWND window, UINT message, WPARAM wparam, LPARAM lparam) {
        if (message == WM_INPUT) {
            RAWINPUT input{};
            UINT size = sizeof(input);
            if (GetRawInputData(reinterpret_cast<HRAWINPUT>(lparam), RID_INPUT,
                                &input, &size, sizeof(RAWINPUTHEADER)) != UINT(-1) &&
                input.header.dwType == RIM_TYPEMOUSE &&
                (input.data.mouse.usButtonFlags & (RI_MOUSE_WHEEL | RI_MOUSE_HWHEEL)))
                wheel_event_.store(true, std::memory_order_relaxed);
            return DefWindowProcW(window, message, wparam, lparam);
        }
        return DefWindowProcW(window, message, wparam, lparam);
    }
    void run_wheel_input() {
        const HINSTANCE instance = GetModuleHandleW(nullptr);
        const std::wstring class_name = L"KeycastRawWheelInput_" + std::to_wstring(GetCurrentThreadId());
        WNDCLASSW cls{};
        cls.lpfnWndProc = raw_window_proc;
        cls.hInstance = instance;
        cls.lpszClassName = class_name.c_str();
        RegisterClassW(&cls);
        HWND window = CreateWindowExW(0, class_name.c_str(), L"", WS_OVERLAPPED,
                                      0, 0, 0, 0, nullptr, nullptr, instance, nullptr);
        if (window) {
            RAWINPUTDEVICE device{};
            device.usUsagePage = 0x01;
            device.usUsage = 0x02;
            device.dwFlags = RIDEV_INPUTSINK;
            device.hwndTarget = window;
            if (RegisterRawInputDevices(&device, 1, sizeof(device))) {
                wheel_thread_id_.store(GetCurrentThreadId(), std::memory_order_release);
                MSG msg{};
                while (GetMessageW(&msg, nullptr, 0, 0) > 0) {
                    TranslateMessage(&msg);
                    DispatchMessageW(&msg);
                }
                device.dwFlags = RIDEV_REMOVE;
                device.hwndTarget = nullptr;
                RegisterRawInputDevices(&device, 1, sizeof(device));
            } else {
                wheel_thread_id_.store(GetCurrentThreadId(), std::memory_order_release);
            }
            DestroyWindow(window);
            UnregisterClassW(class_name.c_str(), instance);
        } else {
            wheel_thread_id_.store(GetCurrentThreadId(), std::memory_order_release);
        }
    }
#endif
    mutable float wheel_hold_ = 0.f;
public:
    WindowsInputPoller() {
#ifdef _WIN32
        wheel_thread_ = std::thread([this] { run_wheel_input(); });
#endif
    }
    ~WindowsInputPoller() {
#ifdef _WIN32
        if (wheel_thread_.joinable()) {
            // Wait until the input thread has created its window/message queue.
            while (wheel_thread_id_.load(std::memory_order_acquire) == 0)
                std::this_thread::yield();
            PostThreadMessageW(wheel_thread_id_.load(std::memory_order_acquire), WM_QUIT, 0, 0);
            wheel_thread_.join();
        }
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
