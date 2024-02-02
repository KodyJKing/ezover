#include <windows.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>

// DX9
#include <d3d9.h>
#pragma comment (lib, "d3d9")

#include "imgui.h"
#include "imgui_impl_win32.h"
#include "imgui_impl_dx9.h"

HWND hWnd;
HINSTANCE hInstance;
int nWidth = 800, nHeight = 600;

UINT resetWidth = 0, resetHeight = 0;

// D3D9 variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3dDevice = NULL;

#define ALPHA_KEY_R 0
#define ALPHA_KEY_G 1
#define ALPHA_KEY_B 0
#define ALPHA_KEY RGB(ALPHA_KEY_R, ALPHA_KEY_G, ALPHA_KEY_B)
#define ALPHA_KEY_DX D3DCOLOR_XRGB(ALPHA_KEY_R, ALPHA_KEY_G, ALPHA_KEY_B)

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

// Returns time in milliseconds since first call.
uint64_t GetTimeSinceStart() {
    static auto start = std::chrono::high_resolution_clock::now();
    auto now = std::chrono::high_resolution_clock::now();
    return std::chrono::duration_cast<std::chrono::milliseconds>(now - start).count();
}

void initDX9(HWND hwnd) {
    // Create D3D9 object
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (d3d == NULL) {
        std::cout << "Couldn't create D3D9 object.\n";
        return;
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN; // Need to use an explicit format with alpha if needing per-pixel alpha composition.
    d3dpp.EnableAutoDepthStencil = TRUE;
    d3dpp.AutoDepthStencilFormat = D3DFMT_D16;
    d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;           // Present with vsync
    //d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE;   // Present without vsync, maximum unthrottled framerate
    if (d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice) < 0) {
        std::cout << "Couldn't create D3D9 device.\n";
        return;
    }

}

void initImgui() {
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    ImGui::StyleColorsDark();
    ImGui_ImplWin32_Init(hWnd);
    ImGui_ImplDX9_Init(d3dDevice);

    // No transparency
    ImGui::GetStyle().Alpha = 1.0f;
}

void OnResize(HWND hWnd, UINT nWidth, UINT nHeight) {
    std::cout << "OnResize" << std::endl;

    if (nWidth == 0 || nHeight == 0)
        return;

    resetWidth = nWidth;
    resetHeight = nHeight;
}

void resetDevice() {
    if (d3dDevice) {
        std::cout << "Resetting device.\n";

        ImGui_ImplDX9_InvalidateDeviceObjects();
        
        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

        if (FAILED(d3dDevice->Reset(&d3dpp))) {
            std::cout << "Couldn't reset device.\n";
            return;
        }
        
        ImGui_ImplDX9_CreateDeviceObjects();

        resetWidth = 0;
        resetHeight = 0;
    }
}

void drawNative() {
    d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    struct Vertex {
        float x, y, z, w;
        DWORD color;
    };

    // Get window size
    RECT rc;
    GetClientRect(hWnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    float cx = width / 2.0f;
    float cy = height / 2.0f;

    // Draw rotating polygon
    const int sides = 6;
    uint64_t t = GetTimeSinceStart();
    float r = 100.0f;
    float dAngle = (float) M_PI * 2.0f / sides;
    float angle = t * 0.0001f;
    Vertex vertices[sides+2];
    vertices[0] = { cx, cy, 0.5f, 1.0f, D3DCOLOR_XRGB(255, 255, 255) };
    static const DWORD colors[3] = {
        D3DCOLOR_XRGB(0, 0, 255),
        D3DCOLOR_XRGB(0, 255, 0),
        D3DCOLOR_XRGB(255, 0, 0),
    };
    for (int i = 1; i < sides+2; i++) {
        float x = cosf(angle + dAngle * i) * r + cx;
        float y = sinf(angle + dAngle * i) * r + cy;
        vertices[i] = { x, y, 0.5f, 1.0f, colors[i % 3]};
    }
    d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLEFAN, sides, vertices, sizeof(Vertex));

    // Draw border lines around window
    d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    float h0 = height - 1.0f;
    float w0 = width - 1.0f;
    DWORD color = D3DCOLOR_XRGB(255, 255, 0);
    Vertex border[] = {
        { 0.0f, 0.0f, 0.5f, 1.0f, color, },
        { w0,   0.0f, 0.5f, 1.0f, color, },
        { w0,   h0,   0.5f, 1.0f, color, },
        { 0.0f, h0,   0.5f, 1.0f, color, },
        { 0.0f, 0.0f, 0.5f, 1.0f, color, },
    };
    d3dDevice->DrawPrimitiveUP(D3DPT_LINESTRIP, 4, border, sizeof(Vertex));
}

void drawMenu() {

    ImGui_ImplDX9_NewFrame();
    ImGui_ImplWin32_NewFrame();
    ImGui::NewFrame();
    ImGui::ShowDemoWindow();
    ImGui::EndFrame();

    d3dDevice->Clear(0, nullptr, D3DCLEAR_TARGET, ALPHA_KEY_DX, 1.0f, 0);
    if (d3dDevice->BeginScene() >= 0) {
        drawNative();

        d3dDevice->SetRenderState(D3DRS_ZENABLE, FALSE);
        d3dDevice->SetRenderState(D3DRS_ALPHABLENDENABLE, FALSE);
        d3dDevice->SetRenderState(D3DRS_SCISSORTESTENABLE, FALSE);
        ImGui::Render();
        ImGui_ImplDX9_RenderDrawData(ImGui::GetDrawData());

        d3dDevice->EndScene();
    }

    d3dDevice->Present(nullptr, nullptr, nullptr, nullptr);
}

int APIENTRY wWinMain(HINSTANCE _hInstance, HINSTANCE hPrevInstance, LPWSTR lpCmdLine, int nCmdShow) {
    std::cout << "Hello DX9" << std::endl;

	hInstance = _hInstance;
	WNDCLASSEX wcex = {
		sizeof(WNDCLASSEX), 
        CS_HREDRAW | CS_VREDRAW,
        WndProc, 
        0, 0, 
        hInstance,
        LoadIcon(NULL, IDI_APPLICATION),
		LoadCursor(NULL, IDC_ARROW),
        (HBRUSH)(COLOR_WINDOW + 1),
        // CreateSolidBrush(ALPHA_KEY),
        NULL, 
        TEXT("WindowClass"),
        NULL,
	};

	if (!RegisterClassEx(&wcex))
		return MessageBox(NULL, TEXT("Cannot register class !"), TEXT("Error"), MB_ICONERROR | MB_OK);

	int nX = (GetSystemMetrics(SM_CXSCREEN) - nWidth) / 2;
    int nY = (GetSystemMetrics(SM_CYSCREEN) - nHeight) / 2;

	hWnd = CreateWindowEx(
        WS_EX_TOPMOST | WS_EX_LAYERED | WS_EX_NOACTIVATE,
        // WS_EX_TOPMOST | WS_EX_NOACTIVATE,
        wcex.lpszClassName,
        TEXT("Test"),
        // WS_OVERLAPPEDWINDOW,
        WS_POPUP,
        nX, nY, nWidth, nHeight, 
        NULL, NULL,
        hInstance,
        NULL
    );

	if (!hWnd)
		return MessageBox(NULL, TEXT("Cannot create window !"), TEXT("Error"), MB_ICONERROR | MB_OK);

    SetLayeredWindowAttributes(hWnd, ALPHA_KEY, 0, LWA_COLORKEY);
    SetWindowLong(hWnd, GWL_EXSTYLE, GetWindowLong(hWnd, GWL_EXSTYLE) | WS_EX_LAYERED);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

    initDX9(hWnd);
    initImgui();

	MSG msg;
	while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if (resetWidth != 0 || resetHeight != 0)
            resetDevice();

        // draw();
        drawMenu();
	}

	return (int)msg.wParam;
}

// Forward declare message handler from imgui_impl_win32.cpp
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {

	switch (message) {
        case WM_SIZE: {
            UINT nWidth = LOWORD(lParam);
            UINT nHeight = HIWORD(lParam);
            OnResize(hWnd, nWidth, nHeight);
            return 0;
        }	
        break;
        case WM_RBUTTONDOWN: {
            std::cout << "Right click.\n";
            return 0;
        }
        break;
        case WM_DESTROY: {
            // Clean();
            PostQuitMessage(0);
            return 0;
        }
        break;
        default:
        
            if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
                return true;

            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}
