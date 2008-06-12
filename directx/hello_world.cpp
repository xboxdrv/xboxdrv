#include <dinput.h>
#include <iostream>
#include <windows.h>
#include <winuser.h>

HWND MainWindowHandle = 0;

LRESULT CALLBACK WndProc(HWND windowHandle,
                         UINT msg,
                         WPARAM wParam,
                         LPARAM lParam)
{
  switch(msg)
    {
      case WM_LBUTTONDOWN:
        //::MessageBox(0, "Hello World", "Hello", MB_OK);
        return 0;

      case WM_KEYDOWN:
        if (wParam == VK_ESCAPE)
          ::DestroyWindow(MainWindowHandle);
        return 0;

      case WM_DESTROY:
        ::PostQuitMessage(0);
        return 0;
    }

  return ::DefWindowProc(windowHandle, msg, wParam, lParam);
}

bool InitWindowsApp(HINSTANCE instanceHandle, int show)
{
  WNDCLASS wc;

  wc.style         = CS_HREDRAW | CS_VREDRAW;
  wc.lpfnWndProc   = WndProc;
  wc.cbClsExtra    = 0;
  wc.cbWndExtra    = 0;
  wc.hInstance     = instanceHandle;
  wc.hIcon         = ::LoadIcon(0, IDI_APPLICATION);
  wc.hCursor       = ::LoadCursor(0, IDC_ARROW);
  wc.hbrBackground = static_cast<HBRUSH>(::GetStockObject(WHITE_BRUSH));
  wc.lpszMenuName  = 0;
  wc.lpszClassName = "Hello";

  if (!::RegisterClass(&wc))
    {
      ::MessageBox(0, "RegisterClass - Failed", 0, 0);
      return false;
    }

  MainWindowHandle = ::CreateWindow("Hello", 
                                    "Hello", 
                                    WS_OVERLAPPEDWINDOW,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    CW_USEDEFAULT,
                                    0,
                                    0,
                                    instanceHandle,
                                    0);

  if (MainWindowHandle == 0)
    {
      ::MessageBox(0, "CreateWindow - Failed", 0, 0);
      return false;
    }

  ::ShowWindow(MainWindowHandle, show);
  ::UpdateWindow(MainWindowHandle);

  return true;
  
}

int Run()
{
  MSG msg;
  ::ZeroMemory(&msg, sizeof(MSG));

  while(::GetMessage(&msg, 0, 0, 0))
    {
      ::TranslateMessage(&msg);
      ::DispatchMessage(&msg);
    }

  return msg.wParam;
}

int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   PSTR      pCmdLine,
                   int       nShowCmd)
{
  HRESULT result = CoInitialize(0);
  LPDIRECTINPUT directinput;

  if (FAILED(result))
    {
      std::cerr << "CL_DisplayWindow_Win32: Damn murphy must hate you. CoInitialize failed!" << std::endl;
    }

  result = CoCreateInstance(CLSID_DirectInput, 0, CLSCTX_INPROC_SERVER, IID_IDirectInput, (LPVOID *) &directinput);
  if (FAILED(result))
    {
      std::cerr << "FAILURE" << std::endl;
    }
  else
    {
      std::cerr << "SUCCESS" << std::endl;
      result = directinput->Initialize(GetModuleHandle(0), DIRECTINPUT_VERSION);
    }

  if (!InitWindowsApp(hInstance, nShowCmd))
    {
      ::MessageBox(0, "Init - Failed", "Error", MB_OK);
      return 0;
    }

  return Run();
}


// EOF //
