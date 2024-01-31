#include <windows.h>
#include <iostream>

// DX9
#include <d3d9.h>
#pragma comment (lib, "d3d9")

HWND hWnd;
HINSTANCE hInstance;
int nWidth = 800, nHeight = 600;

// D3D9 variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3dDevice = NULL;

#define ALPHA_KEY RGB(0,255,0)
#define ALPHA_KEY_DX D3DCOLOR_XRGB(0,255,0)

void OnResize(HWND hWnd, UINT nWidth, UINT nHeight);
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

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

    SetLayeredWindowAttributes(hWnd, ALPHA_KEY, 255, LWA_COLORKEY);

	ShowWindow(hWnd, SW_SHOWNORMAL);
	UpdateWindow(hWnd);

	MSG msg;
	while (GetMessage(&msg, NULL, 0, 0)) {
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}

	return (int)msg.wParam;
}

void draw() {
    d3dDevice->BeginScene();
    d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, ALPHA_KEY, 1.0f, 0);

    d3dDevice->SetFVF(D3DFVF_XYZRHW | D3DFVF_DIFFUSE);
    struct Vertex {
        float x, y, z, w;
        DWORD color;
    };
    Vertex vertices[] = {
        { 150.0f, 50.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 0, 255), },
        { 250.0f, 250.0f, 0.5f, 1.0f, D3DCOLOR_XRGB(0, 255, 0), },
        { 50.0f, 250.0f, 0.5f, 1.0f,  D3DCOLOR_XRGB(255, 0, 0), },
    };
    d3dDevice->DrawPrimitiveUP(D3DPT_TRIANGLELIST, 1, vertices, sizeof(Vertex));

    // Get window size
    RECT rc;
    GetClientRect(hWnd, &rc);
    int width = rc.right - rc.left;
    int height = rc.bottom - rc.top;

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

LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	int wmId, wmEvent;
	switch (message) {
	case WM_CREATE: {
		// HRESULT hr = CoInitialize(NULL);
        
        d3d = Direct3DCreate9(D3D_SDK_VERSION);
        if (d3d == NULL) {
            MessageBox(hWnd, TEXT("Cannot create D3D9 object !"), TEXT("Error"), MB_ICONERROR | MB_OK);
            return -1;
        }

        D3DPRESENT_PARAMETERS d3dpp;
        ZeroMemory(&d3dpp, sizeof(d3dpp));
        d3dpp.Windowed = TRUE;
        d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
        d3dpp.BackBufferFormat = D3DFMT_UNKNOWN;

        if (FAILED(d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &d3dDevice))) {
            MessageBox(hWnd, TEXT("Cannot create D3D9 device !"), TEXT("Error"), MB_ICONERROR | MB_OK);
            return -1;
        }

		return 0;
	}
	break;
	case WM_RBUTTONDOWN: {
        std::cout << "WM_RBUTTONDOWN" << std::endl;
	}
	break;	
	case WM_PAINT: {
		// PAINTSTRUCT ps;
		// HDC hDC = BeginPaint(hWnd, &ps);
		// EndPaint(hWnd, &ps);
        draw();
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
		// CoUninitialize();
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
