#pragma once

#include <windows.h>
#include <mmsystem.h>
#pragma comment(lib, "winmm.lib")

class WaveIn {
public:
	WAVEFORMATEX waveform;
	unsigned char *m_pRecoderBuf;
	int m_recoderNum;
	int m_recoderBufLen;
	WAVEHDR *m_pWaveHdr;
	HWAVEIN m_hWaveIn;
	DWORD_PTR WaveInProc;

	WaveIn(DWORD_PTR);
	~WaveIn();
	void open();
	void start();
	void stop();
};
