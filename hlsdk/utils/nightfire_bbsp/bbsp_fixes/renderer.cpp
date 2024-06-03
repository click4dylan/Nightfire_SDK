
#include "renderer.h"
#include <d3d9.h>
#include <d3dx9.h>
#include <vector>
#include <thread>
#include <stdexcept>
#include <timeapi.h>
#include <mutex>

// Global variables
LPDIRECT3D9 d3d = NULL;
LPDIRECT3DDEVICE9 d3ddev = NULL;
LPDIRECT3DVERTEXBUFFER9 v_buffer = NULL;
HWND hDXWindow = NULL;
bool running = true;
std::thread renderThread;
std::mutex mouseMutex;
// Global flag to track window activation
bool isWindowActive = false;

// Custom vertex format
struct CUSTOMVERTEX {
    FLOAT x, y, z;
    D3DCOLOR color;
};

#define CUSTOMFVF (D3DFVF_XYZ | D3DFVF_DIFFUSE)

// Camera variables
D3DXVECTOR3 cameraPos = D3DXVECTOR3(0.0f, 0.0f, -5.0f); // Camera position
D3DXVECTOR3 cameraLook = D3DXVECTOR3(0.0f, 0.0f, 0.0f); // Camera target
D3DXVECTOR3 cameraUp = D3DXVECTOR3(0.0f, 1.0f, 0.0f); // Camera up vector
float cameraSpeed = 0.1f; // Camera movement speed
float cameraRotationSpeed = 0.002f; // Camera rotation speed
float mouseSensitivityFactor = 0.1f;
int mouse_deltax = 0;
int mouse_deltay = 0;
// Global variables to store accumulated rotation angles
float accumulatedRotationX = 0.0f;
float accumulatedRotationY = 0.0f;


// Triangle data
std::vector<CUSTOMVERTEX> vertices;

// Function prototypes
void InitD3D(HWND hWnd);
void RenderFrame(void);
void CleanD3D(void);
void HandleKeyDown(WPARAM wParam);
void HandleMouseMovement(int x, int y);
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
void MainLoop();

// Function to initialize the scene geometry
void initGeometry()
{
    struct CUSTOMVERTEX { FLOAT x, y, z; DWORD color; };
    CUSTOMVERTEX vertices[] =
    {
        { 3.0f, -3.0f, 0.0f, 0xffff0000 },
        { 0.0f,  3.0f, 0.0f, 0xff00ff00 },
        { -3.0f, -3.0f, 0.0f, 0xff0000ff },
    };

    d3ddev->CreateVertexBuffer(3 * sizeof(CUSTOMVERTEX), 0, D3DFVF_XYZ | D3DFVF_DIFFUSE,
        D3DPOOL_MANAGED, &v_buffer, NULL);

    VOID* pVoid;
    v_buffer->Lock(0, 0, (void**)&pVoid, 0);
    memcpy(pVoid, vertices, sizeof(vertices));
    v_buffer->Unlock();
}

// Initialize Direct3D
void InitD3D(HWND hWnd)
{
    d3d = Direct3DCreate9(D3D_SDK_VERSION);
    if (!d3d) {
        throw std::runtime_error("Failed to create Direct3D interface.");
    }

    D3DPRESENT_PARAMETERS d3dpp;
    ZeroMemory(&d3dpp, sizeof(d3dpp));
    d3dpp.Windowed = TRUE;
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;
    d3dpp.hDeviceWindow = hWnd;

    HRESULT hr = d3d->CreateDevice(D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, hWnd,
        D3DCREATE_HARDWARE_VERTEXPROCESSING, &d3dpp, &d3ddev);

    if (FAILED(hr)) {
        d3d->Release();
        d3d = NULL;

        // Print detailed error message
        std::string errorMsg;
        switch (hr) {
        case D3DERR_INVALIDCALL:
            errorMsg = "D3DERR_INVALIDCALL: Invalid call.";
            break;
        case D3DERR_NOTAVAILABLE:
            errorMsg = "D3DERR_NOTAVAILABLE: This device does not support the queried technique.";
            break;
        case D3DERR_OUTOFVIDEOMEMORY:
            errorMsg = "D3DERR_OUTOFVIDEOMEMORY: Out of video memory.";
            break;
        default:
            errorMsg = "Unknown error.";
            break;
        }

        throw std::runtime_error("Failed to create Direct3D device. Error: " + errorMsg);
    }
    else
    {
        initGeometry();
    }
}

// Render a frame
void RenderFrame(void)
{
    d3ddev->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    if (SUCCEEDED(d3ddev->BeginScene()))
    {
        // Set the vertex format
        d3ddev->SetFVF(D3DFVF_XYZ | D3DFVF_DIFFUSE);

        // Set the stream source
        d3ddev->SetStreamSource(0, v_buffer, 0, sizeof(CUSTOMVERTEX));

        // Set the view transformation
        D3DXMATRIX viewMatrix;
        D3DXMatrixLookAtLH(&viewMatrix, &cameraPos, &cameraLook, &cameraUp);
        d3ddev->SetTransform(D3DTS_VIEW, &viewMatrix);

        // Set the projection transformation (simple perspective projection)
        D3DXMATRIX projectionMatrix;
        D3DXMatrixPerspectiveFovLH(&projectionMatrix, D3DX_PI / 4, 800.0f / 600.0f, 1.0f, 1000.0f);
        d3ddev->SetTransform(D3DTS_PROJECTION, &projectionMatrix);

        // Draw the geometry
        d3ddev->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

        // End the scene
        d3ddev->EndScene();
    }

    d3ddev->Present(NULL, NULL, NULL, NULL);
}

// Clean up Direct3D and COM
void CleanD3D(void)
{
    if (d3ddev)
    {
        d3ddev->Release();
        d3ddev = NULL;
    }
    if (d3d)
    {
        d3d->Release();
        d3d = NULL;
    }
}

// Global variables to track keyboard state
bool keys[256]{};

// Function to handle key down events
void HandleKeyDown(WPARAM wParam)
{
    keys[wParam] = true;
}

// Function to handle key up events
void HandleKeyUp(WPARAM wParam)
{
    keys[wParam] = false;
}

// Global variables to track mouse position
int lastMouseX = 0;
int lastMouseY = 0;
bool first_move = true;

// Function to handle mouse movement
void HandleMouseMovement(int x, int y)
{
    if (!isWindowActive)
        return; // Ignore mouse movements when the window is not active

    static float yaw = 0.0f; // Yaw angle (rotation around Y-axis)
    static float pitch = 0.0f; // Pitch angle (rotation around X-axis)

    // Update yaw and pitch based on mouse movement
    yaw += x * cameraRotationSpeed;
    pitch += y * cameraRotationSpeed;

    // Clamp pitch to avoid flipping
    pitch = max(-D3DX_PI / 2.0f, min(pitch, D3DX_PI / 2.0f));

    // Calculate new camera look direction based on yaw and pitch
    D3DXMATRIX rotationY, rotationX;
    D3DXMatrixRotationY(&rotationY, yaw);
    D3DXMatrixRotationX(&rotationX, pitch);

    D3DXVECTOR3 newLookDir;
    D3DXVECTOR3 d = D3DXVECTOR3(0.0f, 0.0f, 1.0f);

    D3DXMATRIX rot = rotationX * rotationY;
    D3DXVec3TransformNormal(&newLookDir, &d, &rot);

    // Update camera look direction
    cameraLook = cameraPos + newLookDir;
}

// Function to handle window activation
void HandleWindowActivation(WPARAM wParam)
{
    // Update the window activation flag
    isWindowActive = (wParam == WA_ACTIVE || wParam == WA_CLICKACTIVE);
}

// Function to register the window for raw input
void RegisterRawInput(HWND hWnd)
{
    RAWINPUTDEVICE rid;
    rid.usUsagePage = 0x01; // Generic desktop controls
    rid.usUsage = 0x02; // Mouse
    rid.dwFlags = 0;
    rid.hwndTarget = hWnd;

    if (!RegisterRawInputDevices(&rid, 1, sizeof(RAWINPUTDEVICE)))
    {
        // Handle registration failure
    }
}

// Function to handle raw input
void HandleRawInput(HRAWINPUT hRawInput)
{
    RAWINPUT rawInput;
    UINT dataSize = sizeof(rawInput);
    GetRawInputData(hRawInput, RID_INPUT, &rawInput, &dataSize, sizeof(RAWINPUTHEADER));

    if (rawInput.header.dwType == RIM_TYPEMOUSE)
    {
        int deltaX = rawInput.data.mouse.lLastX;
        int deltaY = rawInput.data.mouse.lLastY;

        // Process mouse movement
        HandleMouseMovement(deltaX, deltaY);
    }
}

// Windows procedure
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;

    case WM_KEYDOWN:
        HandleKeyDown(wParam);
        break;

    case WM_KEYUP:
        HandleKeyUp(wParam);
        break;

    case WM_ACTIVATE:
        HandleWindowActivation(wParam);
        break;

#if 0
    case WM_INPUT:
        // Handle raw input
        HandleRawInput(reinterpret_cast<HRAWINPUT>(lParam));
        return 0;
    }
#else
    case WM_MOUSEMOVE:
        // Only handle mouse movement if the window has focus
        if (GetForegroundWindow() == hWnd)
        {
            //HandleMouseMovement(LOWORD(lParam) - (GetSystemMetrics(SM_CXSCREEN) / 2), HIWORD(lParam) - (GetSystemMetrics(SM_CYSCREEN) / 2));
            SetCursorPos(GetSystemMetrics(SM_CXSCREEN) / 2, GetSystemMetrics(SM_CYSCREEN) / 2);
        }
        break;

    //case WM_TIMER:
    //    HandleTimer(wParam);
    //    break;
    }
#endif

    return DefWindowProc(hWnd, message, wParam, lParam);
}

// Main loop for rendering
void MainLoop()
{
    // Variables for timing
    DWORD prevTime = timeGetTime();
    float deltaTime = 0.0f;
    timeBeginPeriod(1);

    MSG msg;
    while (running)
    {
        // Calculate delta time
        DWORD currentTime = timeGetTime();
        deltaTime = (currentTime - prevTime) / 1000.0f; // Convert to seconds
        prevTime = currentTime;

        while (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);

            if (msg.message == WM_QUIT)
            {
                running = false;
                break;
            }
        }

        if (running)
        {
            RenderFrame();
        }
    }

    CleanD3D();
}

// Start rendering
//extern "C" __declspec(dllexport) void StartRendering()
DWORD WINAPI StartRendering(LPVOID lParam)
{
    Sleep(500);

    try {
        // Register window class
        WNDCLASSEX wc;
        ZeroMemory(&wc, sizeof(WNDCLASSEX));
        wc.cbSize = sizeof(WNDCLASSEX);
        wc.style = CS_HREDRAW | CS_VREDRAW;
        wc.lpfnWndProc = WindowProc;
        wc.hInstance = GetModuleHandle(NULL);
        wc.hCursor = LoadCursor(NULL, IDC_ARROW);
        wc.lpszClassName = "WindowClass";

        RegisterClassEx(&wc);

        // Create window
        hDXWindow = CreateWindowEx(NULL, "WindowClass", "DirectX Example", WS_OVERLAPPEDWINDOW, 300, 300, 800, 600, NULL, NULL, GetModuleHandle(NULL), NULL);
        if (!hDXWindow) {
            throw std::runtime_error("Failed to create window.");
        }

        ShowWindow(hDXWindow, SW_SHOW);

        // Initialize Direct3D
        InitD3D(hDXWindow);

        // Start the main rendering loop in a separate thread
        //renderThread = std::thread(MainLoop);

        // Register the window for raw input
        //RegisterRawInput(hDXWindow);

        MainLoop();

#if 0
        for (;;)
        {
            Sleep(1);
        }
#endif
    }
    catch (const std::exception& e) {
        MessageBoxA(NULL, e.what(), "Error", MB_OK | MB_ICONERROR);
    }
}

// Stop rendering
extern "C" __declspec(dllexport) void StopRendering()
{
    running = false;
    if (renderThread.joinable())
    {
        renderThread.join();
    }
    DestroyWindow(hDXWindow);
    UnregisterClass("WindowClass", GetModuleHandle(NULL));
}