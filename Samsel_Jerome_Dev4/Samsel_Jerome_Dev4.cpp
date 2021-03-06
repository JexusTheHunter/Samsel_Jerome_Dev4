// Samsel_Jerome_Dev4.cpp : Defines the entry point for the application.
//

#include "framework.h"
#include "Samsel_Jerome_Dev4.h"

#include <d3d11.h>
#pragma comment (lib, "d3d11.lib")

#include "MyVshader.csh"
#include "MyPshader.csh"

#include <DirectXMath.h>
using namespace DirectX;

#include "MyMeshVshader.csh"

#include "Assets/StoneHenge.h"

ID3D11Device* myDev;
IDXGISwapChain* mySwap;
ID3D11DeviceContext* myCon;
ID3D11RenderTargetView* myRtv;
D3D11_VIEWPORT myPort;
float aspectRatio = 1;

struct MyVertex
{
    float xyzw[4];
    float rgba[4];
};
unsigned int numVerts = 0;

struct CameraChange
{
    float posX, posY, posZ, lookX, lookY, lookZ, yaw, pitch;
};

CameraChange cInput = { 0.0f, 5.0f, 0.0f,
                        0.0f, 5.0f, 1.0f, 
                        0.0f, 0.0f };

ID3D11Buffer* vBuff;
ID3D11InputLayout* vLayout;
ID3D11VertexShader* vShader;
ID3D11PixelShader* pShader;

ID3D11Buffer* cBuff;
ID3D11Buffer* vBuffMesh;
ID3D11Buffer* iBuffMesh;

ID3D11VertexShader* vMeshShader;
ID3D11InputLayout* vMeshLayout;

ID3D11Texture2D* zBuffer;
ID3D11DepthStencilView* zBufferView;

struct WVP
{
    XMFLOAT4X4 wMatrix;
    XMFLOAT4X4 vMatrix;
    XMFLOAT4X4 pMatrix;
}MyMatricies;

#define MAX_LOADSTRING 100

// Global Variables:
HINSTANCE hInst;                                // current instance 
WCHAR szTitle[MAX_LOADSTRING];                  // The title bar text
WCHAR szWindowClass[MAX_LOADSTRING];            // the main window class name

// Forward declarations of functions included in this code module:
ATOM                MyRegisterClass(HINSTANCE hInstance);
BOOL                InitInstance(HINSTANCE, int);
LRESULT CALLBACK    WndProc(HWND, UINT, WPARAM, LPARAM);
INT_PTR CALLBACK    About(HWND, UINT, WPARAM, LPARAM);

CameraChange CheckCamera(CameraChange);

int APIENTRY wWinMain(_In_ HINSTANCE hInstance,
                     _In_opt_ HINSTANCE hPrevInstance,
                     _In_ LPWSTR    lpCmdLine,
                     _In_ int       nCmdShow)
{
    UNREFERENCED_PARAMETER(hPrevInstance);
    UNREFERENCED_PARAMETER(lpCmdLine);

    // TODO: Place code here.

    // Initialize global strings
    LoadStringW(hInstance, IDS_APP_TITLE, szTitle, MAX_LOADSTRING);
    LoadStringW(hInstance, IDC_SAMSELJEROMEDEV4, szWindowClass, MAX_LOADSTRING);
    MyRegisterClass(hInstance);

    // Perform application initialization:
    if (!InitInstance (hInstance, nCmdShow))
    {
        return FALSE;
    }

    HACCEL hAccelTable = LoadAccelerators(hInstance, MAKEINTRESOURCE(IDC_SAMSELJEROMEDEV4));

    MSG msg;

    // Main message loop:
    while (true)//GetMessage(&msg, nullptr, 0, 0)) waits for message not good
    {
        PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE);
        if (!TranslateAccelerator(msg.hwnd, hAccelTable, &msg))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }
        if (msg.message == WM_QUIT)
            break;

        //rendering here
        //ID3D11RenderTargetView* tempRTV[] = { myRtv };
        //myCon->OMSetRenderTargets(1, tempRTV, nullptr);

        float color[] = { 0, 1, 1, 1, };
        myCon->ClearRenderTargetView(myRtv, color);

        myCon->ClearDepthStencilView(zBufferView, D3D11_CLEAR_DEPTH, 1, 0);

        ID3D11RenderTargetView* tempRtv[] = { myRtv };
        myCon->OMSetRenderTargets(1, tempRtv, zBufferView);
        myCon->RSSetViewports(1, &myPort);
        myCon->IASetInputLayout(vLayout);

        UINT strides[] = {sizeof(MyVertex)};
        UINT offsets[] = { 0 };
        ID3D11Buffer* tempVB[] = { vBuff };
        myCon->IASetVertexBuffers(0, 1, tempVB, strides, offsets);
        myCon->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

        myCon->VSSetShader(vShader, 0, 0);
        myCon->PSSetShader(pShader, 0, 0);

        static float rot = 0; rot += 0.0001f;
        XMMATRIX temp = XMMatrixIdentity();
        temp = XMMatrixTranslation(3, 2, -5);
        XMMATRIX temp2 = XMMatrixRotationY(rot);
        temp = XMMatrixMultiply(temp2, temp);
        XMStoreFloat4x4(&MyMatricies.wMatrix, temp);

        cInput = CheckCamera(cInput);

        temp = XMMatrixLookAtLH({cInput.posX, cInput.posY, cInput.posZ}, {cInput.lookX, cInput.lookY, cInput .lookZ}, {0, 1, 0});
        XMStoreFloat4x4(&MyMatricies.vMatrix, temp);
        temp = XMMatrixPerspectiveFovLH(3.14f/2.0f, aspectRatio, 0.1f, 1000);
        XMStoreFloat4x4(&MyMatricies.pMatrix, temp);

        D3D11_MAPPED_SUBRESOURCE gpuBuffer;
        HRESULT hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
        *((WVP*)(gpuBuffer.pData)) = MyMatricies;
        //memcpy(gpuBuffer.pData, &MyMatricies, sizeof(WVP));
        myCon->Unmap(cBuff, 0);

        ID3D11Buffer* constants[] = { cBuff };
        myCon->VSSetConstantBuffers(0, 1, constants);

        myCon->Draw(numVerts, 0);

        UINT mesh_strides[] = { sizeof(_OBJ_VERT_) };
        UINT mesh_offsets[] = { 0 };
        ID3D11Buffer* meshVB[] = { vBuffMesh };
        myCon->IASetVertexBuffers(0, 1, meshVB, mesh_strides, mesh_offsets);
        myCon->IASetIndexBuffer(iBuffMesh, DXGI_FORMAT_R32_UINT, 0);
        myCon->VSSetShader(vMeshShader, 0, 0);
        myCon->IASetInputLayout(vMeshLayout);

        temp = XMMatrixIdentity();
        XMStoreFloat4x4(&MyMatricies.wMatrix, temp);
        hr = myCon->Map(cBuff, 0, D3D11_MAP_WRITE_DISCARD, 0, &gpuBuffer);
        *((WVP*)(gpuBuffer.pData)) = MyMatricies;
        myCon->Unmap(cBuff, 0);

        myCon->DrawIndexed(2532, 0, 0);

        mySwap->Present(0, 0);
    }

    //release all our d3d11 interfaces
    myRtv->Release();
    vBuff->Release();
    myCon->Release();
    mySwap->Release();
    vShader->Release();
    pShader->Release();
    vLayout->Release();
    myDev->Release();


    return (int) msg.wParam;
}
  

//
//  FUNCTION: MyRegisterClass()
//
//  PURPOSE: Registers the window class.
//
ATOM MyRegisterClass(HINSTANCE hInstance)
{
    WNDCLASSEXW wcex;

    wcex.cbSize = sizeof(WNDCLASSEX);

    wcex.style          = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc    = WndProc;
    wcex.cbClsExtra     = 0;
    wcex.cbWndExtra     = 0;
    wcex.hInstance      = hInstance;
    wcex.hIcon          = LoadIcon(hInstance, MAKEINTRESOURCE(IDI_SAMSELJEROMEDEV4));
    wcex.hCursor        = LoadCursor(nullptr, IDC_ARROW);
    wcex.hbrBackground  = (HBRUSH)(COLOR_WINDOW+1);
    wcex.lpszMenuName   = MAKEINTRESOURCEW(IDC_SAMSELJEROMEDEV4);
    wcex.lpszClassName  = szWindowClass;
    wcex.hIconSm        = LoadIcon(wcex.hInstance, MAKEINTRESOURCE(IDI_SMALL));

    return RegisterClassExW(&wcex);
}

//
//   FUNCTION: InitInstance(HINSTANCE, int)
//
//   PURPOSE: Saves instance handle and creates main window
//
//   COMMENTS:
//
//        In this function, we save the instance handle in a global variable and
//        create and display the main program window.
//
BOOL InitInstance(HINSTANCE hInstance, int nCmdShow)
{
   hInst = hInstance; // Store instance handle in our global variable

   HWND hWnd = CreateWindowW(szWindowClass, szTitle, WS_OVERLAPPEDWINDOW,
      CW_USEDEFAULT, 0, CW_USEDEFAULT, 0, nullptr, nullptr, hInstance, nullptr);

   if (!hWnd)
   {
      return FALSE;
   }

   ShowWindow(hWnd, nCmdShow);
   UpdateWindow(hWnd);

   RECT myWinR;
   GetClientRect(hWnd, &myWinR);

   //attach d3 to window 
   D3D_FEATURE_LEVEL dx11 = D3D_FEATURE_LEVEL_11_0;
   DXGI_SWAP_CHAIN_DESC swap;
   ZeroMemory(&swap, sizeof(DXGI_SWAP_CHAIN_DESC));
   swap.BufferCount = 1;
   swap.OutputWindow = hWnd;
   swap.Windowed = true;
   swap.SwapEffect = DXGI_SWAP_EFFECT_DISCARD;
   swap.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
   swap.BufferDesc.Width = myWinR.right - myWinR.left;
   swap.BufferDesc.Height = myWinR.bottom - myWinR.top;
   swap.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
   swap.SampleDesc.Count = 1;

   aspectRatio = swap.BufferDesc.Width / float(swap.BufferDesc.Height);

   HRESULT hr;
   hr = D3D11CreateDeviceAndSwapChain(   NULL, D3D_DRIVER_TYPE_HARDWARE, NULL, D3D11_CREATE_DEVICE_DEBUG,
                                    &dx11, 1, D3D11_SDK_VERSION, &swap, &mySwap, &myDev, 0, &myCon);


   ID3D11Resource* backbuffer;
   hr = mySwap->GetBuffer(0, __uuidof(backbuffer), (void**)&backbuffer);
   hr = myDev->CreateRenderTargetView(backbuffer, NULL, &myRtv);
   
   backbuffer->Release();

   myPort.Width = swap.BufferDesc.Width;
   myPort.Height = swap.BufferDesc.Height;
   myPort.TopLeftX = myPort.TopLeftY = 0;
   myPort.MinDepth = 0;
   myPort.MaxDepth = 1;

   MyVertex tri[] =
   {
       {{ 0,      1.0f,   0,     1}, {1, 1, 1, 1}},
       {{ 0.25f, -0.25f, -0.25f, 1}, {1, 0, 1, 1}},
       {{-0.25f, -0.25f, -0.25f, 1}, {1, 1, 0, 1}},

       {{ 0,      1.0f,   0,     1}, {1, 1, 1, 1}},
       {{ 0.25f, -0.25f,  0.25f, 1}, {1, 0, 1, 1}},
       {{ 0.25f, -0.25f, -0.25f, 1}, {1, 1, 0, 1}},

       {{ 0,      1.0f,   0,     1}, {1, 1, 1, 1}},
       {{-0.25f, -0.25f,  0.25f, 1}, {1, 0, 1, 1}},
       {{ 0.25f, -0.25f,  0.25f, 1}, {1, 1, 0, 1}},

       {{ 0,      1.0f,   0,     1}, {1, 1, 1, 1}},
       {{-0.25f, -0.25f, -0.25f, 1}, {1, 0, 1, 1}},
       {{-0.25f, -0.25f,  0.25f, 1}, {1, 1, 0, 1}}
   };

   numVerts = ARRAYSIZE(tri);

   D3D11_BUFFER_DESC bDesc;
   D3D11_SUBRESOURCE_DATA subData;
   ZeroMemory(&bDesc, sizeof(bDesc));
   ZeroMemory(&subData, sizeof(subData));

   bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   bDesc.ByteWidth = sizeof(MyVertex) * numVerts;
   bDesc.CPUAccessFlags = 0;
   bDesc.MiscFlags = 0;
   bDesc.StructureByteStride = 0;
   bDesc.Usage = D3D11_USAGE_IMMUTABLE;

   subData.pSysMem = tri;

   hr = myDev->CreateBuffer(&bDesc, &subData, &vBuff);

   hr = myDev->CreateVertexShader(MyVshader, sizeof(MyVshader), nullptr, &vShader);
   hr = myDev->CreatePixelShader(MyPshader, sizeof(MyPshader), nullptr, &pShader);

   D3D11_INPUT_ELEMENT_DESC ieDesc[] =
   {
       {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA},
       {"COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 16, D3D11_INPUT_PER_VERTEX_DATA},
   };

   hr = myDev->CreateInputLayout(ieDesc, 2, MyVshader, sizeof(MyVshader), &vLayout);

   ZeroMemory(&bDesc, sizeof(bDesc));

   bDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
   bDesc.ByteWidth = sizeof(WVP);
   bDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
   bDesc.MiscFlags = 0;
   bDesc.StructureByteStride = 0;
   bDesc.Usage = D3D11_USAGE_DYNAMIC;

   hr = myDev->CreateBuffer(&bDesc, &subData, &cBuff);

   bDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
   bDesc.ByteWidth = sizeof(StoneHenge_data);
   bDesc.CPUAccessFlags = 0;
   bDesc.MiscFlags = 0;
   bDesc.StructureByteStride = 0;
   bDesc.Usage = D3D11_USAGE_IMMUTABLE;

   subData.pSysMem = StoneHenge_data;
   hr = myDev->CreateBuffer(&bDesc, &subData, &vBuffMesh);

   bDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
   bDesc.ByteWidth = sizeof(StoneHenge_indicies);
   subData.pSysMem = StoneHenge_indicies;
   hr = myDev->CreateBuffer(&bDesc, &subData, &iBuffMesh);

   hr = myDev->CreateVertexShader(MyMeshVshader, sizeof(MyMeshVshader), nullptr, &vMeshShader);

   D3D11_INPUT_ELEMENT_DESC meshInputDesc[] =
   {
       {"POSITION", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA},
       {"TEXCOORD", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D11_INPUT_PER_VERTEX_DATA},
       {"NORMAL", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 24, D3D11_INPUT_PER_VERTEX_DATA},
   };

   hr = myDev->CreateInputLayout(meshInputDesc, 3, MyMeshVshader, sizeof(MyMeshVshader), &vMeshLayout);

   D3D11_TEXTURE2D_DESC zDesc;
   ZeroMemory(&zDesc, sizeof(zDesc));
   zDesc.ArraySize = 1;
   zDesc.BindFlags = D3D11_BIND_DEPTH_STENCIL;
   zDesc.Width = swap.BufferDesc.Width;
   zDesc.Height = swap.BufferDesc.Height;
   zDesc.Format = DXGI_FORMAT_D32_FLOAT;
   zDesc.Usage = D3D11_USAGE_DEFAULT;
   zDesc.MipLevels = 1;
   zDesc.SampleDesc.Count = 1;

   hr = myDev->CreateTexture2D(&zDesc, nullptr, &zBuffer);
   hr = myDev->CreateDepthStencilView(zBuffer, nullptr, &zBufferView);

   return TRUE;
}

//
//  FUNCTION: WndProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE: Processes messages for the main window.
//
//  WM_COMMAND  - process the application menu
//  WM_PAINT    - Paint the main window
//  WM_DESTROY  - post a quit message and return
//
//
LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_COMMAND:
        {
            int wmId = LOWORD(wParam);
            // Parse the menu selections:
            switch (wmId)
            {
            case IDM_ABOUT:
                DialogBox(hInst, MAKEINTRESOURCE(IDD_ABOUTBOX), hWnd, About);
                break;
            case IDM_EXIT:
                DestroyWindow(hWnd);
                break;
            default:
                return DefWindowProc(hWnd, message, wParam, lParam);
            }
        }
        break;
    case WM_PAINT:
        {
            PAINTSTRUCT ps;
            HDC hdc = BeginPaint(hWnd, &ps);
            // TODO: Add any drawing code that uses hdc here...
            EndPaint(hWnd, &ps);
        }
        break;
    case WM_DESTROY:
        PostQuitMessage(0);
        break;
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
    return 0;
}

// Message handler for about box.
INT_PTR CALLBACK About(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam)
{
    UNREFERENCED_PARAMETER(lParam);
    switch (message)
    {
    case WM_INITDIALOG:
        return (INT_PTR)TRUE;

    case WM_COMMAND:
        if (LOWORD(wParam) == IDOK || LOWORD(wParam) == IDCANCEL)
        {
            EndDialog(hDlg, LOWORD(wParam));
            return (INT_PTR)TRUE;
        }
        break;
    }
    return (INT_PTR)FALSE;
}

CameraChange CheckCamera(CameraChange input)
{
    CameraChange output = input;
    if (GetAsyncKeyState(0x57))
    {
        output.posZ += 0.01f;
        output.lookZ += 0.01f;
    }
    if (GetAsyncKeyState(0x53))
    {
        output.posZ -= 0.01f;
        output.lookZ -= 0.01f;
    }
    if (GetAsyncKeyState(0x41))
    {
        output.posX -= 0.01f;
        output.lookX -= 0.01f;
    }
    if (GetAsyncKeyState(0x44))
    {
        output.posX += 0.01f;
        output.lookX += 0.01f;
    }

    //struct currentPitch
    //{
    //    float relativeY = 0 , relativeX = 1;
    //};

    //currentPitch cPit;

    //if (GetAsyncKeyState(VK_UP))
    //{
    //    if (output.pitch >= 90.0f)
    //    {
    //        output.pitch = 90.0f;
    //    }
    //}
    //if (GetAsyncKeyState(VK_DOWN))
    //{
    //    if (output.pitch <= -90.0f)
    //    {
    //        output.pitch = -90.0f;
    //    }
    //}

    //output.pitch = 45.0f;

    //float pitch = output.pitch / 90.0f;

    //if (output.pitch > 0)
    //{
    //    cPit.relativeY = pitch;
    //    cPit.relativeX = 1 - pitch;
    //}



    if (GetAsyncKeyState(VK_RIGHT))
    {
        if (output.yaw > 360.0f)
        {
            output.yaw -= 360.0f;
        }
    }
    if (GetAsyncKeyState(VK_LEFT))
    {
        if (output.yaw < 360.0f)
        {
            output.yaw += 360.0f;
        }
    }



    return output;
}