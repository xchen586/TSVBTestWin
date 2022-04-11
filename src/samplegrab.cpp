

#include "stdafx.h"
#include "samplegrab.h"
#include "CVBHandler.h"
#include "CVBAutoLock.h"

CVBHandler * _pVBHandler = NULL;
IPipeline * _pPipeline = NULL;
IFrameFactory * _pFrameFactroy = NULL;

AM_MEDIA_TYPE m_mtGrab;

IBaseFilter *_pGrabberFilter = NULL;
ISampleGrabber *_pGrabber = NULL;

long _bufferSize = 0;
unsigned char* _pBufferCapture = 0;
unsigned char* _pBufferVBInput = 0;
unsigned char* _pBufferVBResult = 0;

Gdiplus::Bitmap *_pCapturedBitmap = 0;
Gdiplus::Bitmap *_pVBBitmap = 0;
unsigned int gWidth = 0;
unsigned int gHeight = 0;
unsigned int gChannels = 0;


int sgSetBitmapData(Gdiplus::Bitmap* pBitmap, const unsigned char* pData);
void sgFlipUpDown(unsigned char* pData);
void sgFreeMediaType(AM_MEDIA_TYPE& mt);


void sgSetVBHandler(CVBHandler * pVBHandler)
{
	_pVBHandler = pVBHandler;
	if (_pVBHandler) {
		_pPipeline = _pVBHandler->_pipeline;
		_pFrameFactroy = _pVBHandler->_frameFactory;
	}
}

BOOL isVBReady()
{
	return _pVBHandler 
		&& _pVBHandler->isInitialized()
		&& _pPipeline
		&& _pFrameFactroy;
}

IBaseFilter* sgGetSampleGrabber()
{
	return _pGrabberFilter;
}

void sgCloseSampleGrabber()
{
	if (_pBufferCapture != 0) {
		delete[] _pBufferCapture;
		_pBufferCapture = 0;
		_bufferSize = 0;
	}

	if (_pBufferVBInput) {
		delete[] _pBufferVBInput;
		_pBufferVBInput = 0;
	}

	if (_pBufferVBResult) {
		delete[] _pBufferVBResult;
		_pBufferVBResult = 0;
	}

	if (_pCapturedBitmap != 0) {
		::delete _pCapturedBitmap;
		_pCapturedBitmap = 0;
	}

	SAFE_RELEASE(_pGrabberFilter);
	SAFE_RELEASE(_pGrabber);

	gWidth = 0;
	gHeight = 0;
	gChannels = 0;
}

HRESULT sgAddSampleGrabber(IGraphBuilder *pGraph)
{
	// Create the Sample Grabber.
	HRESULT hr = CoCreateInstance(CLSID_SampleGrabber, NULL, CLSCTX_INPROC_SERVER,
		IID_IBaseFilter, (void**)& _pGrabberFilter);
	if (FAILED(hr)) {
		return hr;
	}
	hr = pGraph->AddFilter(_pGrabberFilter, L"Sample Grabber");
	if (FAILED(hr)) {
		return hr;
	}

	_pGrabberFilter->QueryInterface(IID_ISampleGrabber, (void**)&_pGrabber);
	return hr;
}

Gdiplus::PixelFormat sgGetBitmapPixelFormat()
{
	Gdiplus::PixelFormat ret = PixelFormat32bppRGB;
	if (MEDIASUBTYPE_ARGB32 == m_mtGrab.subtype) {
		ret = PixelFormat32bppPARGB;
	}
	return ret;
}

HRESULT sgSetSampleGrabberMediaType()
{

	ZeroMemory(&m_mtGrab, sizeof(AM_MEDIA_TYPE));
	m_mtGrab.majortype = MEDIATYPE_Video;
	m_mtGrab.subtype = MEDIASUBTYPE_RGB32;
	//m_mtGrab.subtype = MEDIASUBTYPE_ARGB32;
	HRESULT hr = _pGrabber->SetMediaType(&m_mtGrab);
	if (FAILED(hr)) {
		return hr;
	}
	hr = _pGrabber->SetOneShot(FALSE);
	hr = _pGrabber->SetBufferSamples(TRUE);
	return hr;
}

HRESULT sgGetSampleGrabberMediaType()
{
	AM_MEDIA_TYPE mt;
	HRESULT hr = _pGrabber->GetConnectedMediaType(&mt);
	if (FAILED(hr)) {
		return hr;
	}

	VIDEOINFOHEADER *pVih = (VIDEOINFOHEADER *)mt.pbFormat;
	gChannels = pVih->bmiHeader.biBitCount / 8;
	gWidth = pVih->bmiHeader.biWidth;
	gHeight = pVih->bmiHeader.biHeight;

	sgFreeMediaType(mt);
	return hr;
}

Gdiplus::Bitmap *sgGetCaptureBitmap()
{
	/*if (pGrabber == 0 || pBuffer == 0 || gChannels != 3)
			return 0;*/

	if (_pGrabber == 0 || _pBufferCapture == 0 || gChannels != 4) {
		return 0;
	}

	//sgRgb32ToRgba(_pBufferCapture, gWidth * gHeight);
	Gdiplus::PixelFormat pixelFormat = sgGetBitmapPixelFormat();
	if (_pCapturedBitmap == 0)
		_pCapturedBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, pixelFormat);     
	else if (gWidth != _pCapturedBitmap->GetWidth() || gHeight != _pCapturedBitmap->GetHeight()) {
		::delete _pCapturedBitmap;
		_pCapturedBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, pixelFormat);
	}

	if (_bufferSize != gWidth * gHeight * gChannels)
		return 0;

	if (sgSetBitmapData(_pCapturedBitmap, _pBufferCapture) == 0)
		return _pCapturedBitmap;
	else
		return 0;
}

Gdiplus::Bitmap *sgGetVBBitmap(unsigned char * pBufferInput)
{
	if (!pBufferInput) {
		return 0;
	}

	Gdiplus::PixelFormat pixelFormat = sgGetBitmapPixelFormat();
	if (_pVBBitmap == 0) {
		_pVBBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, pixelFormat);
	}
	else if (gWidth != _pVBBitmap->GetWidth() || gHeight != _pVBBitmap->GetHeight()) {
		::delete _pVBBitmap;
		_pVBBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, pixelFormat);
	}

	if (sgSetBitmapData(_pVBBitmap, pBufferInput) == 0) {
		return _pVBBitmap;
	}
	else {
		return 0;
	}
}

Gdiplus::Bitmap *sgGetVBBitmap() 
{
	unsigned char * pVBData = sgGetVBResultData();
	if (!pVBData) {
		return 0;
	}

	return sgGetVBBitmap(pVBData);
}

unsigned char* sgGrabCaptureRawData()
{

	HRESULT hr;

	if (_pGrabber == 0)
		return 0;

	long Size = 0;
	hr = _pGrabber->GetCurrentBuffer(&Size, NULL);
	if (FAILED(hr)) {
		return 0;
	}
	else if (Size != _bufferSize) {
		_bufferSize = Size;
		if (_pBufferCapture != 0) {
			delete[] _pBufferCapture;
			_pBufferCapture = nullptr;
		}
		_pBufferCapture = new unsigned char[_bufferSize];
		if (_pBufferVBInput != 0) {
			delete[] _pBufferVBInput;
			_pBufferVBInput = nullptr;
		}
		_pBufferVBInput = new unsigned char[_bufferSize];
		if (_pBufferVBResult != 0) {
			delete[] _pBufferVBResult;
			_pBufferVBResult = nullptr;
		}
		_pBufferVBResult = new unsigned char[_bufferSize];
	}
	if (!_pBufferCapture) {
		return 0;
	}
	hr = _pGrabber->GetCurrentBuffer(&_bufferSize, (long*)_pBufferCapture);
	if (FAILED(hr))
		return 0;
	else {
		sgFlipUpDown(_pBufferCapture);
		return _pBufferCapture;
	}
}

void sgConvertBetweenCaptureAndVB(const unsigned char* input, int pixel_width, unsigned char* output)
{
	if (MEDIASUBTYPE_ARGB32 == m_mtGrab.subtype) {
		sgConvertBetweenBGRAandARGB(input, pixel_width, output);
	}
	else {
		sgConvertBetweenBGRAandRGBA(input, pixel_width, output);
	}
}

unsigned char* sgGrabVBInputData()
{
	unsigned char * pRGB32Data = sgGrabCaptureRawData();

	if (pRGB32Data && _pBufferVBInput) {
		sgConvertBetweenCaptureAndVB(pRGB32Data, gWidth * gHeight, _pBufferVBInput);
		return _pBufferVBInput;
	}
	else {
		return 0;
	}
}

long sgGetBufferSize()
{
	return _bufferSize;
}

int sgSetBitmapData(Gdiplus::Bitmap* pBitmap, const unsigned char* pData)
{
	Gdiplus::BitmapData bitmapData;
	bitmapData.Width = pBitmap->GetWidth();
	bitmapData.Height = pBitmap->GetHeight();
	//bitmapData.Stride = 3 * bitmapData.Width;
	bitmapData.Stride = 4 * bitmapData.Width;
	bitmapData.PixelFormat = pBitmap->GetPixelFormat();
	bitmapData.Scan0 = (VOID*)pData;
	bitmapData.Reserved = NULL;

	Gdiplus::Status s = pBitmap->LockBits(&Gdiplus::Rect(0, 0, pBitmap->GetWidth(), pBitmap->GetHeight()),
		Gdiplus::ImageLockModeWrite | Gdiplus::ImageLockModeUserInputBuf,
		PixelFormat32bppPARGB, &bitmapData); //PixelFormat24bppRGB, PixelFormat32bppRGB
	if (s == Gdiplus::Ok)
		pBitmap->UnlockBits(&bitmapData);

	return s;
}

void sgSwap(unsigned char str1, unsigned char str2)
{
	unsigned char temp = str1;
	str1 = str2;
	str2 = temp;
}

void sgFlipUpDown(unsigned char* pData)
{
	unsigned char* scan0 = pData;
	unsigned char* scan1 = pData + ((gWidth * gHeight * gChannels) - (gWidth * gChannels));

	for (unsigned int y = 0; y < gHeight / 2; y++) {
		for (unsigned int x = 0; x < gWidth * gChannels; x++) {
			swap(scan0[x], scan1[x]);
			//sgSwap(scan0[x], scan1[x]);
		}
		scan0 += gWidth * gChannels;
		scan1 -= gWidth * gChannels;
	}

}

unsigned int sgGetDataWidth()
{
	return gWidth;
}

unsigned int sgGetDataHeight()
{
	return gHeight;
}

unsigned int sgGetDataChannels()
{
	return gChannels;
}

void sgFreeMediaType(AM_MEDIA_TYPE& mt)
{
	if (mt.cbFormat != 0)
	{
		CoTaskMemFree((PVOID)mt.pbFormat);
		mt.cbFormat = 0;
		mt.pbFormat = NULL;
	}
	if (mt.pUnk != NULL)
	{
		// Unecessary because pUnk should not be used, but safest.
		mt.pUnk->Release();
		mt.pUnk = NULL;
	}
}

void sgConvertBetweenBGRAandRGBA(const unsigned char* input, int pixel_width, unsigned char* output)
{
	for (int x = 0; x < pixel_width; x++) {
		const unsigned char* pixel_in = &input[x * 4];
		unsigned char* pixel_out = &output[x * 4];
		pixel_out[0] = pixel_in[2];
		pixel_out[1] = pixel_in[1];
		pixel_out[2] = pixel_in[0];
		pixel_out[3] = pixel_in[3];
	}
}

void sgConvertBetweenBGRAandARGB(const unsigned char* input, int pixel_width, unsigned char* output)
{
	for (int x = 0; x < pixel_width; x++) {
		const unsigned char* pixel_in = &input[x * 4];
		unsigned char* pixel_out = &output[x * 4];
		pixel_out[0] = pixel_in[3];
		pixel_out[1] = pixel_in[2];
		pixel_out[2] = pixel_in[1];
		pixel_out[3] = pixel_in[0];
	}
}


void sgConvertRGB32toRGBA(unsigned char* input, int pixel_width)
{
	for (int x = 0; x < pixel_width; x++) {
		unsigned char* pixel_in = &input[x * 4];
		pixel_in[3] = 0;
	}
}

BOOL sgGetBufferDataFromFrame(IFrame * pFrame, unsigned char* pBuffer, BOOL isCopy = FALSE)
{
	if (!pFrame
		||!pBuffer) {
		return FALSE;
	}
	
	ILockedFrameData * pFrameData = nullptr;
	pFrameData = pFrame->lock(FrameLock::read);
	if (!pFrameData) {
		return FALSE;
	}

	void * pDataPointer = pFrameData->dataPointer(0);
	if (!pDataPointer) {
		pFrameData->release();
		return FALSE;
	}

	size_t width = pFrame->width();
	size_t height = pFrame->height();
	size_t bytePerLine = pFrameData->bytesPerLine(0);
	size_t bytePerPixel = bytePerLine / width;
	
	//memcpy(pBuffer, pDataPointer, width * height * bytePerPixel);
	sgConvertBetweenCaptureAndVB((unsigned char*)(pDataPointer), width * height, pBuffer);
	pFrameData->release();

	return TRUE;
}

unsigned char* sgGetVBResultData(unsigned char* pInputBGRA)
{
	if (!pInputBGRA) {
		return 0;
	}

	if (!_pBufferVBResult) {
		return 0;
	}

	if (!isVBReady()) {
		return 0;
	}

	void * pInput = (void *)pInputBGRA;
	IFrame * pInputFrame = _pFrameFactroy->createBGRA(pInput
		, gWidth * gChannels
		, gWidth
		, gHeight
		, true);
	if (!pInputFrame) {
		return 0;
	}

	PipelineError err;
	IFrame * pProcessedFrame = _pPipeline->process(pInputFrame, &err);
	if (pInputFrame) {
		pInputFrame->release();
	}
	
	if (!pProcessedFrame) {
		return 0;
	}
	
	unsigned char * ret = _pBufferVBResult;
	BOOL canGetData = sgGetBufferDataFromFrame(pProcessedFrame, ret);
	if (pProcessedFrame) {
		pProcessedFrame->release();
	}

	if (canGetData) {
		return ret;
	}
	else {
		return 0;
	}
}

unsigned char* sgGetVBResultData()
{
	unsigned char* pInputBGRA = sgGrabVBInputData();
	if (pInputBGRA) {
		return sgGetVBResultData(pInputBGRA);
	}
	else {
		return 0;
	}
	
}