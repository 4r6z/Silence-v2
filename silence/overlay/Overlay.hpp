#pragma once

#include "imgui/imgui.h"
#include <windows.h>
#include <string>
#include <vector>
#include <d3d11.h>
#pragma comment(lib, "d3d11.lib")

extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

struct Screen {
    std::int32_t x;
    std::int32_t y;
    std::int32_t width;
    std::int32_t height;

    bool operator==(const Screen& screen) const {
        return memcmp(this, &screen, sizeof(Screen)) == 0;
    }

    bool operator!=(const Screen& screen) const {
        return !(*this == screen);
    }
};

class Overlay {
public:
    static ID3D11Device* d3d11Device; // this needs to be public so I can access it from my loadtexture function
    static bool Render();
    static void Move_Window(HWND hWnd);
    static bool FullScreen(HWND windowHandle);

private:
    static bool CreateDeviceD3D(HWND hWnd);
    static void CleanupDeviceD3D();
    static void CreateRenderTarget();
    static void CleanupRenderTarget();
    static LRESULT WINAPI WndProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

private:
    static ID3D11DeviceContext* d3d11DeviceContext;
    static IDXGISwapChain* dxgiSwapChain;
    static ID3D11RenderTargetView* d3d11RenderTargetView;
    static bool initialized;
    static bool assetsloaded;
};
