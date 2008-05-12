
#define DIRECTINPUT_VERSION 0x0800
#define INITGUID

#include <windows.h>
#include <dinput.h>
#include <iostream>

#ifdef _MSC_VER
#pragma comment(lib, "dinput8.lib")
#endif

BOOL CALLBACK enum_devices_callback(LPCDIDEVICEINSTANCE lpddi, LPVOID pvRef);

int main(int, char**)
{
	IDirectInput8 *directinput;
	HRESULT result = DirectInput8Create(GetModuleHandle(0), DIRECTINPUT_VERSION, IID_IDirectInput8, (LPVOID *) &directinput, 0);
	if (FAILED(result))
		std::cout << "Unable to initialize direct input" << std::endl;

	std::cout << "Enumerating DI8DEVCLASS_GAMECTRL devices.." << std::endl;
	result = directinput->EnumDevices(
		DI8DEVCLASS_GAMECTRL,
		&enum_devices_callback,
		directinput,
		DIEDFL_ATTACHEDONLY);
	if (FAILED(result))
		std::cout << "Unable to enumerate game controllers" << std::endl;
	else
		std::cout << "Enumeration complete" << std::endl;

	directinput->Release();

	return 0;
}

BOOL CALLBACK enum_devices_callback(LPCDIDEVICEINSTANCE device_instance, LPVOID pvRef)
{
	IDirectInput8 *directinput = reinterpret_cast<IDirectInput8 *>(pvRef);

	std::wcout << L"Found device, product name='" << device_instance->tszProductName << L"', instance name='" << device_instance->tszInstanceName << L"'" << std::endl;

	IDirectInputDevice8 *directinput_device = 0;
	HRESULT result = directinput->CreateDevice(
		device_instance->guidInstance,
		&directinput_device,
		0);
	if (SUCCEEDED(result))
	{
		DIDEVCAPS capabilities;
		capabilities.dwSize = sizeof(DIDEVCAPS);
		result = directinput_device->GetCapabilities(&capabilities);
		if (SUCCEEDED(result))
		{
			std::cout << "Device has " << capabilities.dwAxes << " axes, " << capabilities.dwButtons << " buttons and " << capabilities.dwPOVs << " point-of-view controllers" << std::endl;
		}

		directinput_device->Release();
	}

	return TRUE;
}
