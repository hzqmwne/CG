#include "waveIn.h"
#include <iostream>

WaveIn::WaveIn(DWORD_PTR callbackFunction) {
	waveform.wFormatTag = WAVE_FORMAT_PCM;//声音格式为PCM  
	waveform.nSamplesPerSec = 8000;//采样率，16000次/秒  
	waveform.wBitsPerSample = 16;//采样比特，16bits/次  
	waveform.nChannels = 1;//采样声道数，2声道  
	waveform.nAvgBytesPerSec = 16000;//每秒的数据率，就是每秒能采集多少字节的数据  
	waveform.nBlockAlign = 2;//一个块的大小，采样bit的字节数乘以声道数  
	waveform.cbSize = 0;//一般为0  

	m_pRecoderBuf = new unsigned char[20 * 1024];
	m_recoderNum = 20;
	m_recoderBufLen = 20 * 1024;
	m_pWaveHdr = new WAVEHDR[20];
	memset(m_pRecoderBuf, 0, 20 * 1024);
	memset(m_pWaveHdr, 0, sizeof(WAVEHDR) * 20);
	
	WaveInProc = callbackFunction;
}

WaveIn::~WaveIn() {
	delete[] m_pRecoderBuf;
	delete[] m_pWaveHdr;
}

void WaveIn::open() {
	MMRESULT mmres = waveInOpen(&m_hWaveIn, WAVE_MAPPER, &waveform, (DWORD_PTR)WaveInProc, (DWORD_PTR)this, CALLBACK_FUNCTION);
	if (mmres != MMSYSERR_NOERROR) {
		// failed, try again.
		return;
	}
	for (int i = 0; i < m_recoderNum; ++i) {
		m_pWaveHdr[i].lpData = (char*)m_pRecoderBuf + i * 1024;
		m_pWaveHdr[i].dwBufferLength = 1024;
		MMRESULT mmres = waveInPrepareHeader(m_hWaveIn, &m_pWaveHdr[i], sizeof(WAVEHDR));
		mmres = waveInAddBuffer(m_hWaveIn, &m_pWaveHdr[i], sizeof(WAVEHDR));
	}
}

void WaveIn::start() {
	MMRESULT mmres = waveInStart(m_hWaveIn);
}

void WaveIn::stop() {
	MMRESULT mmres = waveInStop(m_hWaveIn);
	mmres = waveInClose(m_hWaveIn);
}
