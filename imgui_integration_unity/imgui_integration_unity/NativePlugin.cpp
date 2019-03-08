#pragma comment(lib, "d3dcompiler.lib")

#include <d3d11.h>

#include "IUnityInterface.h"
#include "IUnityGraphics.h"
#include "IUnityGraphicsD3D11.h"
#include "imgui.h"
#include "imgui_impl_dx11.h"
#include "imgui_impl_win32.h"

extern LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
WNDPROC oldWndProc = nullptr;
HWND unityHandle = nullptr;

static float facialWeights[100];
static int facialWeightCount = 100;

LRESULT CALLBACK MyWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	if (ImGui_ImplWin32_WndProcHandler(hWnd, message, wParam, lParam))
		return true;
	
	// otherwise let unity process it.	
	return CallWindowProc(oldWndProc, hWnd, message, wParam, lParam);
}

extern "C" void UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API SetFacialWeights(int count, float* weights)
{
	facialWeightCount = count;
	for (int i = 0; i < count; ++i)
	{
		facialWeights[i] = weights[i];
	}
}

namespace
{
	IUnityInterfaces* UnityInterfaces = nullptr;
	IUnityGraphics* UnityGraphics = nullptr;
	UnityGfxRenderer UnityGfxRendererType = kUnityGfxRendererNull;

	ID3D11Device* D3D11Device = nullptr;
	ID3D11DeviceContext* D3D11DeviceContext = nullptr;
	
	void OnGraphicsDeviceEvent(const UnityGfxDeviceEventType EventType)
	{
		switch (EventType)
		{
		case kUnityGfxDeviceEventInitialize:
		{
			// get renderer type. ex : kUnityGfxRendererD3D11, kUnityGfxRendererOpenGLES20 and so on)
			UnityGfxRendererType = UnityGraphics->GetRenderer();
						
			auto* D3D = UnityInterfaces->Get<IUnityGraphicsD3D11>();
			D3D11Device = D3D->GetDevice();
			D3D11Device->GetImmediateContext(&D3D11DeviceContext);
			
			ImGui::CreateContext();
			ImGui::StyleColorsDark();

			// get unity window handle.			
#ifdef UNITY_EDITOR
			HWND rootHandle = FindWindow(L"UnityContainerWndClass", nullptr);
			if (rootHandle != nullptr)
			{
				unityHandle = FindWindowEx(rootHandle, nullptr, L"UnityGUIViewWndClass", L"UnityEditor.GameView");							
			}
			else
			{
				MessageBox(rootHandle, TEXT("Can't found rootHandle"), TEXT("Caption"), MB_OK);
			}
#else
			unityHandle = GetActiveWindow();
#endif
			
			// get wndproc
			oldWndProc = (WNDPROC)SetWindowLongPtr(unityHandle, GWLP_WNDPROC, (LONG_PTR)MyWndProc);

			if (oldWndProc == nullptr)
			{
				MessageBox(unityHandle, TEXT("oldWndProc is nullptr"), TEXT("Caption"), MB_OK);
			}

			ImGui_ImplWin32_Init(unityHandle);
			ImGui_ImplDX11_Init(D3D11Device, D3D11DeviceContext);

			for (int i = 0; i < 100; ++i)
			{
				facialWeights[i] = 0;
			}
			break;
		}
		case kUnityGfxDeviceEventShutdown:
			UnityGfxRendererType = kUnityGfxRendererNull;

			if (oldWndProc != nullptr)
			{
				 SetWindowLongPtr(unityHandle, GWLP_WNDPROC, (LONG_PTR)oldWndProc);
				 oldWndProc = nullptr;
			}		

			ImGui_ImplDX11_Shutdown();
			ImGui_ImplWin32_Shutdown();
			D3D11DeviceContext->Release();
			ImGui::DestroyContext();

			break;
		case kUnityGfxDeviceEventBeforeReset:
			break;
		case kUnityGfxDeviceEventAfterReset:
			break;
		default:;
		}
	}
}

void UnityPluginLoad(IUnityInterfaces* UnityInterfacesPtr)
{
	UnityInterfaces = UnityInterfacesPtr;
	UnityGraphics = UnityInterfaces->Get<IUnityGraphics>();
		
	UnityGraphics->RegisterDeviceEventCallback(OnGraphicsDeviceEvent);

	OnGraphicsDeviceEvent(kUnityGfxDeviceEventInitialize);
}

void UnityPluginUnload()
{
	UnityGraphics->UnregisterDeviceEventCallback(OnGraphicsDeviceEvent);
}

static void UNITY_INTERFACE_API OnRenderEvent(int eventID)
{
	ImGui_ImplDX11_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("GXLab");
	ImGui::Text("Face weights");
	ImGui::PlotHistogram("weights", facialWeights, facialWeightCount, 0, NULL, 0.0f, 100.0f, ImVec2(0, 150));
	ImGui::Text("Expression : Happy");

	ImGui::Text("MousePos : %d %d", (int)ImGui::GetIO().MousePos.x, (int)ImGui::GetIO().MousePos.y);

	ImGui::End();
	
	ImGui::Render();
	
	ImGui_ImplDX11_RenderDrawData(ImGui::GetDrawData());
}

extern "C" UnityRenderingEvent UNITY_INTERFACE_EXPORT UNITY_INTERFACE_API GetRenderEventFunc()
{
	return OnRenderEvent;
}
