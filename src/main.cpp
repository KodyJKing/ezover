#include <windows.h>
#include <iostream>
#define _USE_MATH_DEFINES
#include <math.h>
#include <chrono>

// DX9
#include <d3d9.h>
#pragma comment (lib, "d3d9")

HWND hWnd;
HINSTANCE hInstance;
int nWidth = 800, nHeight = 600;

// D3D9 variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3dDevice = NULL;

#define ALPHA_KEY_R 0
#define ALPHA_KEY_G 1
#define ALPHA_KEY_B 0
#define ALPHA_KEY RGB(ALPHA_KEY_R, ALPHA_KEY_G, ALPHA_KEY_B)
#define ALPHA_KEY_DX D3DCOLOR_XRGB(ALPHA_KEY_R, ALPHA_KEY_G, ALPHA_KEY_B)

void OnResize(HWND hWnd, UINT nWidth, UINT nHeight);
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
        MessageBox(hwnd, TEXT("Cannot create D3D9 object !"), TEXT("Error"), MB_ICONERROR | MB_OK);
        return;
    }

    // Create D3D9 device
    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

    if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice))) {
        MessageBox(hwnd, TEXT("Cannot create D3D9 device !"), TEXT("Error"), MB_ICONERROR | MB_OK);
        return;
    }
}

void draw() {
    // Get precise time
    uint64_t t = GetTimeSinceStart();

    // d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0,0,255), 1.0f, 0);
    d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, ALPHA_KEY_DX, 1.0f, 0);

    d3dDevice->BeginScene();

    // Get window size
    RECT rc;
    GetClientRect(hWnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

    float cx = width / 2.0f;
    float cy = height / 2.0f;

    d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    struct Vertex {
        float x, y, z, w;
        DWORD color;
    };

    float r = 100.0f;
    float pi = (float) M_PI;
    const int sides = 6;
    float dAngle = pi * 2.0f / sides;
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

    d3dDevice->EndScene();
    d3dDevice->Present(NULL, NULL, NULL, NULL);
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
        CreateSolidBrush(ALPHA_KEY),
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
        // WS_EX_TOPMOST | WS_EX_LAYERED,
        wcex.lpszClassName,
        TEXT("Test"),
        WS_OVERLAPPEDWINDOW,
        // WS_POPUP,
        nX, nY, nWidth, nHeight, 
        NULL, NULL,
        hInstance,
        NULL
    );

	if (!hWnd)
		return MessageBox(NULL, TEXT("Cannot create window !"), TEXT("Error"), MB_ICONERROR | MB_OK);

    SetLayeredWindowAttributes(hWnd, ALPHA_KEY, 0, LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

    initDX9(hWnd);

	MSG msg;
	while (true) {
        if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
            if (msg.message == WM_QUIT)
                break;
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        draw();
	}

	return (int)msg.wParam;
}

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
        case WM_RBUTTONDOWN: {
            std::cout << "Right click." << std::endl;
        }
        break;	
        case WM_SIZE: {
            UINT nWidth = LOWORD(lParam);
            UINT nHeight = HIWORD(lParam);
            OnResize(hWnd, nWidth, nHeight);
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
            return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

void OnResize(HWND hWnd, UINT nWidth, UINT nHeight) {
    std::cout << "OnResize" << std::endl;

    // Skip if window is minimized
    if (nWidth == 0 || nHeight == 0)
        return;

    if (d3dDevice) {
        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

        if (FAILED(d3dDevice->Reset(&d3dpp))) {
            MessageBox(hWnd, TEXT("Cannot reset D3D9 device !"), TEXT("Error"), MB_ICONERROR | MB_OK);
            return;
        }
    }
}
