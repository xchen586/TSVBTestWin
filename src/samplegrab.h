
#ifndef SAMPLEGRAB_h
#define SAMPLEGRAB_h

#define SAFE_RELEASE(x) { if (x) x->Release(); x = NULL; }

class CVBHandler;

void sgSetVBHandler(CVBHandler * pVBHandler);

IBaseFilter* sgGetSampleGrabber();
HRESULT sgAddSampleGrabber(IGraphBuilder *pGraph);
HRESULT sgSetSampleGrabberMediaType();
HRESULT sgGetSampleGrabberMediaType();

unsigned char* sgGrabRGB32Data();            //call grab data first
Gdiplus::Bitmap* sgGetCaptureBitmap();        //fill bitmap with grabbed data
unsigned char* sgGrabBGRAData();
Gdiplus::Bitmap *sgGetVBBitmap(unsigned char * pBufferInput);


unsigned char* sgGetVBResultData();
Gdiplus::Bitmap *sgGetVBBitmap();

long sgGetBufferSize();

unsigned int sgGetDataWidth();
unsigned int sgGetDataHeight();
unsigned int sgGetDataChannels();

void sgCloseSampleGrabber();

void sgConvertBetweenBGRAandRGBA(const unsigned char* input, int pixel_width, unsigned char* output);
void sgRgb32ToRgba(unsigned char* input, int pixel_width);

#endif