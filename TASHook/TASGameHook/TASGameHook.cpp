// TASGameHook.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <windows.h>
#include <tlhelp32.h>
#include <cinttypes>
#include <vector>
#include <unordered_map>
#include <cstdio>

uint64_t ADDR_INPUT_TICK				= 0x006F1D8C;
uint64_t ADDR_KEYBOARD_INPUT			= 0x006B1620;
uint64_t ADDR_FRAMES_SINCE_LEVEL_START	= 0x00746F88;
uint64_t ADDR_WINDOW_FOCUS				= 0x00721E8C;
uint64_t ADDR_MAP_STRING				= 0x006A8174;
uint64_t ADDR_SUBLEVEL_INDICATOR		= 0x00746F90;
uint64_t ADDR_UPDOWNVIEW				= 0x402AD4BC;
uint64_t ADDR_LEFTRIGHTVIEW				= 0x402AD4B8;
uint64_t ADDR_LEFTMOUSE					= 0x006B1818;
uint64_t ADDR_RIGHTMOUSE				= 0x006B181A;

uint8_t inputBuffer[104];

DWORD haloProcessID = 0;

// KEYS is a layout of the memory 
enum KEYS {
	ESC = 0,
	F1,	F2,
	F3,
	F4,
	F5,
	F6,
	F7,
	F8,
	F9,
	F10,
	F11,
	F12,
	PrntScrn,
	ScrollLock,
	PauseBreak,
	Tilde,
	NUM_1,
	NUM_2,
	NUM_3,
	NUM_4,
	NUM_5,
	NUM_6,
	NUM_7,
	NUM_8,
	NUM_9,
	NUM_0,
	Minus,
	Equal,
	Backspace,
	Tab,
	Q,
	W,
	E,
	R,
	T,
	Y,
	U,
	I,
	O,
	P,
	LeftBracket,
	RightBracket,
	Pipe,
	CapsLock,
	A,
	S,
	D,
	F,
	G,
	H,
	J,
	K,
	L,
	Colon,
	Quote,
	Enter,
	LShift,
	Z,
	X,
	C,
	V,
	B,
	N,
	M,
	Comma,
	Period,
	ForwardSlash,
	RShift,
	LCtrl,
	LWin,
	LAlt,
	Space,
	RAlt,
	RWin,
	KeyThatLiterallyNoOneHasEverUsed,
	RightCtrl,
	Up,
	Down,
	Left,
	Right
};

//char KEY_PRINT_CODES[] = {
//	'E',
//	'1',
//	'2',
//	F3,
//	F4,
//	F5,
//	F6,
//	F7,
//	F8,
//	F9,
//	F10,
//	F11,
//	F12,
//	PrntScrn,
//	ScrollLock,
//	PauseBreak,
//	Tilde,
//	NUM_1,
//	NUM_2,
//	NUM_3,
//	NUM_4,
//	NUM_5,
//	NUM_6,
//	NUM_7,
//	NUM_8,
//	NUM_9,
//	NUM_0,
//	Minus,
//	Equal,
//	Backspace,
//	Tab,
//	'Q',
//	'W',
//	'E',
//	'R',
//	'T',
//	'Y',
//	'U',
//	'I',
//	'O',
//	'P',
//	LeftBracket,
//	RightBracket,
//	Pipe,
//	CapsLock,
//	'A',
//	'S',
//	'D',
//	'F',
//	'G',
//	'H',
//	'J',
//	'K',
//	'L',
//	Colon,
//	Quote,
//	Enter,
//	LShift,
//	'Z',
//	'X',
//	'C',
//	'V',
//	'B',
//	'N',
//	'M',
//	Comma,
//	Period,
//	ForwardSlash,
//	RShift,
//	LCtrl,
//	LWin,
//	LAlt,
//	'_',
//	RAlt,
//	RWin,
//	KeyThatLiterallyNoOneHasEverUsed,
//	RightCtrl,
//	Up,
//	Down,
//	Left,
//	Right
//};

bool GetHaloPID() {
	PROCESSENTRY32 entry;
	entry.dwSize = sizeof(PROCESSENTRY32);

	HANDLE snapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (Process32First(snapshot, &entry) == TRUE)
	{
		while (Process32Next(snapshot, &entry) == TRUE)
		{
			if (_tcsicmp(entry.szExeFile, _T("halo.exe")) == 0)
			{
				haloProcessID = entry.th32ProcessID;
				CloseHandle(snapshot);
				return true;
			}
		}
	}

	CloseHandle(snapshot);
	return false;
}

void ReadProcMem(uint64_t loc, void* buf, SIZE_T size) {
	
	HANDLE pHandle;
	//SYSTEM_INFO si;
	//MEMORY_BASIC_INFORMATION mbi;
	LPVOID lpMem;
	DWORD totalRead;

	pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, haloProcessID);
	if (pHandle == NULL) {
		haloProcessID = -1;
		return; 
	}

	lpMem = (void*)loc;

	ReadProcessMemory(pHandle, lpMem, (LPVOID)(buf), size, &totalRead);

	CloseHandle(pHandle);
}

void WriteProcMem(uint64_t loc, void* buf, SIZE_T size) {
	HANDLE pHandle;
	//SYSTEM_INFO si;
	//MEMORY_BASIC_INFORMATION mbi;
	LPVOID lpMem;
	DWORD totalWritten;

	pHandle = OpenProcess(PROCESS_ALL_ACCESS, 0, haloProcessID);
	if (pHandle == NULL) {
		haloProcessID = -1;
		return;
	}

	lpMem = (void*)loc;

	WriteProcessMemory(pHandle, lpMem, (LPVOID)buf, size, &totalWritten);
}

struct InputMoment {
	uint8_t inputBuf[104];
	float updown, leftright;
	uint8_t leftmouse, rightmouse;
};

//struct InputKey {
//	uint32_t sublevel;
//	uint32_t inputCounter;
//};

union InputKey {
	uint64_t fullKey;
	struct {
		uint32_t sublevel;
		uint32_t inputCounter;
	};
};

int main()
{
	std::unordered_map<uint64_t, InputMoment> allInputs;

	bool isRecording = false;
	bool isPlayback = false;
	int counter = 0;

	int32_t lastInputCounter = -1;
	while (true) {

		counter++;
		if (counter % 10 == 0)
		{
			printf("%d\n", counter);
		}

		while (GetHaloPID() == false) {
			printf("PID for halo.exe not found. Start the game plz...\n");
			Sleep(1000);
		}

		uint32_t inputCounter = 0;
		uint32_t sublevel;

		char mapstring[16];

		ReadProcMem(ADDR_MAP_STRING, &mapstring, sizeof(mapstring));
		ReadProcMem(ADDR_INPUT_TICK, &inputCounter, sizeof(inputCounter));
		ReadProcMem(ADDR_SUBLEVEL_INDICATOR, &sublevel, sizeof(sublevel));

		if (inputCounter == lastInputCounter) {
			continue;
		}
		if (lastInputCounter != -1 && inputCounter != 2 && inputCounter > lastInputCounter + 1) {
			printf("WARNING: Skipped an input trigger!\n");
		}
		lastInputCounter = inputCounter;

		InputMoment im;
		ReadProcMem(ADDR_KEYBOARD_INPUT, &(im.inputBuf), sizeof(im.inputBuf));
		ReadProcMem(ADDR_UPDOWNVIEW, &(im.updown), sizeof(im.updown));
		ReadProcMem(ADDR_LEFTRIGHTVIEW, &(im.leftright), sizeof(im.leftright));
		ReadProcMem(ADDR_LEFTMOUSE, &(im.leftmouse), sizeof(im.leftmouse));
		ReadProcMem(ADDR_RIGHTMOUSE, &(im.rightmouse), sizeof(im.rightmouse));

		InputKey ik;
		ik.sublevel = sublevel;
		ik.inputCounter = lastInputCounter;

		if (im.inputBuf[F1] > 1 && !isRecording) {
			allInputs.clear();
			isRecording = false;
			printf("Recording Started\n");
			im.inputBuf[F1] = 0;
		} 
		if (im.inputBuf[F2] > 1 && (isRecording || isPlayback)) {
			isRecording = false;
			isPlayback = false;
			printf("Recording/Playback Stopped\n");
			im.inputBuf[F2] = 0;
		}
		if (im.inputBuf[F3] > 1 && !isPlayback) {
			isPlayback = true;
			printf("Playback Start\n");
			im.inputBuf[F3] = 0;
		}
		
		if (lastInputCounter == 0) {
			//Sleep(1);
			uint32_t frameCounter;
			ReadProcMem(ADDR_FRAMES_SINCE_LEVEL_START, &frameCounter, sizeof(frameCounter));

			if (allInputs.size() == 0) {
				isRecording = true;
				printf("Recording Started\n");
			}

			if (frameCounter <= 1) {
				printf("Restart Level\n");
			}
			else {
				printf("Load Checkpoint\n");
			}
		}

		if (isPlayback && allInputs.count(ik.fullKey) >= 1) {
			//printf("Replaying Input %u-%d\n", ik.sublevel, ik.inputCounter);
			//ik.inputCounter -= 1;
			if (allInputs.count(ik.fullKey)) {
				InputMoment savedIM = allInputs[ik.fullKey];
				WriteProcMem(ADDR_KEYBOARD_INPUT, &savedIM.inputBuf, sizeof(savedIM.inputBuf));
				WriteProcMem(ADDR_UPDOWNVIEW, &savedIM.updown, sizeof(savedIM.updown));
				WriteProcMem(ADDR_LEFTRIGHTVIEW, &savedIM.leftright, sizeof(savedIM.leftright));
				WriteProcMem(ADDR_LEFTMOUSE, &savedIM.leftmouse, sizeof(savedIM.leftmouse));
				WriteProcMem(ADDR_RIGHTMOUSE, &savedIM.rightmouse, sizeof(savedIM.rightmouse));
			}
			continue;
		}

		if (isRecording) {
			allInputs[ik.fullKey] = im;
			printf("Recording ");
		}


		printf("%u-%d:",ik.sublevel,lastInputCounter);

		/*if (ik.inputCounter == 1099) {
			InputKey ikk;
			ikk.fullKey = ik.fullKey;
			ikk.inputCounter = 1088;
			allInputs[ik.fullKey] = allInputs[ikk.fullKey];
		}*/
		
		char output[105];
		output[104] = 0;
		/*for (int i = 0; i < sizeof(im.inputBuf); i++) {
			output[i] = (im.inputBuf[i] == 0) ? 0x20 : KEY_PRINT_CODES[i];
		}*/
		printf("%s\n", output);
	}
    return 0;
}

