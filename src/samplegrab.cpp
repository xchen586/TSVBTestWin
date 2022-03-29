

#include "stdafx.h"
#include "samplegrab.h"
#include "CVBHandler.h"
#include "CVBAutoLock.h"

CVBHandler * _pVBHandler = NULL;
IBaseFilter *_pGrabberFilter = NULL;
ISampleGrabber *_pGrabber = NULL;

long _bufferSize = 0;
unsigned char* _pBufferCapture = 0;
unsigned char* _pBufferVB = 0;

Gdiplus::Bitmap *_pCapturedBitmap = 0;
Gdiplus::Bitmap *_pVBBitmap = 0;
unsigned int gWidth = 0;
unsigned int gHeight = 0;
unsigned int gChannels = 0;


int sgSetBitmapData(Gdiplus::Bitmap* pBitmap, const unsigned char* pData);
void sgFlipUpDown(unsigned char* pData);
void sgFreeMediaType(AM_MEDIA_TYPE& mt);


void setVBHandler(CVBHandler * pVBHandler)
{
	_pVBHandler = pVBHandler;
}

BOOL isVBReady()
{
	return _pVBHandler && _pVBHandler->isInitialized();
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
	
		if (_pBufferVB) {
			delete[] _pBufferVB;
			_pBufferVB = 0;
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
                                      IID_IBaseFilter, (void**) & _pGrabberFilter);
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

HRESULT sgSetSampleGrabberMediaType()
{
        AM_MEDIA_TYPE mt;
        ZeroMemory(&mt, sizeof(AM_MEDIA_TYPE));
        mt.majortype = MEDIATYPE_Video;
        //mt.subtype = MEDIASUBTYPE_RGBA32 NV12;
		//mt.subtype = MEDIASUBTYPE_RGB24;
		mt.subtype = MEDIASUBTYPE_RGB32;
        HRESULT hr = _pGrabber->SetMediaType(&mt);
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

        if (_pCapturedBitmap == 0)                
                _pCapturedBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, PixelFormat32bppRGB);  //PixelFormat24bppRGB, PixelFormat32bppRGB    
        else if (gWidth != _pCapturedBitmap->GetWidth() || gHeight != _pCapturedBitmap->GetHeight()) {
                ::delete _pCapturedBitmap;
                _pCapturedBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, PixelFormat32bppRGB); //PixelFormat24bppRGB, PixelFormat32bppRGB
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
	if (_pVBBitmap == 0)
		_pVBBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, PixelFormat32bppRGB);  
	else if (gWidth != _pVBBitmap->GetWidth() || gHeight != _pVBBitmap->GetHeight()) {
		::delete _pVBBitmap;
		_pVBBitmap = ::new Gdiplus::Bitmap(gWidth, gHeight, PixelFormat32bppRGB); 
	}

	if (sgSetBitmapData(_pVBBitmap, pBufferInput) == 0) {
		return _pVBBitmap;
	}
	else {
		return 0;
	}
}

unsigned char* sgGrabRGB32Data()
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
				if (_pBufferVB != 0) {
					delete[] _pBufferVB;
					_pBufferVB = nullptr;
 				}
				_pBufferVB = new unsigned char[_bufferSize];
        }
		if (!_pBufferCapture) {
			return 0;
		}
        hr = _pGrabber->GetCurrentBuffer(&_bufferSize, (long*)_pBufferCapture);
        if (FAILED(hr))
                return 0;
        else {
                sgFlipUpDown(_pBufferCapture);
				sgRgb32ToRgba(_pBufferCapture, gWidth * gHeight);
				//sgConvertBetweenBGRAandRGBA(_pBufferCapture, gWidth * gHeight, _pBufferVB);
				return _pBufferCapture; 
        }
}

unsigned char* sgGrabBGRAData()
{
	unsigned char * pRGB32Data = sgGrabRGB32Data();
	if (pRGB32Data && _pBufferVB) {
		sgConvertBetweenBGRAandRGBA(pRGB32Data, gWidth * gHeight, _pBufferVB);
		return _pBufferVB;
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
				PixelFormat32bppRGB, &bitmapData); //PixelFormat24bppRGB, PixelFormat32bppRGB
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


void sgRgb32ToRgba(unsigned char* input, int pixel_width)
{
	for (int x = 0; x < pixel_width; x++) {
		unsigned char* pixel_in = &input[x * 4];
		pixel_in[3] = 0;
	}
}