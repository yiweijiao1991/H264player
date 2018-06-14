// dllmain.cpp : Defines the entry point for the DLL application.
#include "stdafx.h"
#include "GFFX.h"
#include "H264Show.h"
BOOL APIENTRY DllMain1( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
					 )
{
	switch (ul_reason_for_call)
	{
	case DLL_PROCESS_ATTACH:
	case DLL_THREAD_ATTACH:
	case DLL_THREAD_DETACH:
	case DLL_PROCESS_DETACH:
		break;
	}
	return TRUE;
}

HANDLE __stdcall H264_Play(HWND hWnd, LPCSTR szURL)
{
	CGFFX::Initialize();
	CGFFX* gffx = new CGFFX();
	gffx->Play(hWnd, szURL);
	return gffx;
}

VOID __stdcall H264_Destroy(HANDLE hPlay)
{
	CGFFX* gffx = (CGFFX *)hPlay;
	if(NULL != gffx)
	{
		delete gffx;
	}
}
VOID __stdcall H264_Control(HANDLE hPlay,int cmd)
{
	CGFFX* gffx = (CGFFX *)hPlay;
	if(NULL != gffx)
	{
		gffx->H264control(cmd);
	}
}