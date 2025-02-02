#include "SPIFFSGIFPlayer.h"

int x_offset, y_offset, LEDPANEL_WIDTH_GIF, LEDPANEL_HEIGHT_GIF;
unsigned long start_tick = 0;
MatrixPanel_I2S_DMA *display_gif;
AnimatedGIF gif;
File gif_file;

void gif_player_init(MatrixPanel_I2S_DMA *display_placeholder, int width, int height) {
    display_gif = display_placeholder;
    gif.begin(LITTLE_ENDIAN_PIXELS);
    LEDPANEL_WIDTH_GIF = width;
    LEDPANEL_HEIGHT_GIF = height;
}

// Draw a line of image directly on the LED Matrix
void GIFDraw(GIFDRAW *pDraw)
{
    uint8_t *s;
    uint16_t *d, *usPalette, usTemp[320];
    int x, y, iWidth;

  iWidth = pDraw->iWidth;
  if (iWidth > LEDPANEL_WIDTH_GIF)
      iWidth = LEDPANEL_WIDTH_GIF;

    usPalette = pDraw->pPalette;
    y = pDraw->iY + pDraw->y; // current line
    
    s = pDraw->pPixels;
    if (pDraw->ucDisposalMethod == 2) // restore to background color
    {
      for (x=0; x<iWidth; x++)
      {
        if (s[x] == pDraw->ucTransparent)
           s[x] = pDraw->ucBackground;
      }
      pDraw->ucHasTransparency = 0;
    }
    // Apply the new pixels to the main image
    if (pDraw->ucHasTransparency) // if transparency used
    {
      uint8_t *pEnd, c, ucTransparent = pDraw->ucTransparent;
      int x, iCount;
      pEnd = s + pDraw->iWidth;
      x = 0;
      iCount = 0; // count non-transparent pixels
      while(x < pDraw->iWidth)
      {
        c = ucTransparent-1;
        d = usTemp;
        while (c != ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent) // done, stop
          {
            s--; // back up to treat it like transparent
          }
          else // opaque
          {
             *d++ = usPalette[c];
             iCount++;
          }
        } // while looking for opaque pixels
        if (iCount) // any opaque pixels?
        {
          for(int xOffset = 0; xOffset < iCount; xOffset++ ){
            display_gif->drawPixel(x + xOffset, y, usTemp[xOffset]); // 565 Color Format
          }
          x += iCount;
          iCount = 0;
        }
        // no, look for a run of transparent pixels
        c = ucTransparent;
        while (c == ucTransparent && s < pEnd)
        {
          c = *s++;
          if (c == ucTransparent)
             iCount++;
          else
             s--; 
        }
        if (iCount)
        {
          x += iCount; // skip these
          iCount = 0;
        }
      }
    }
    else // does not have transparency
    {
      s = pDraw->pPixels;
      // Translate the 8-bit pixels through the RGB565 palette (already byte reversed)
      for (x=0; x<pDraw->iWidth; x++)
      {
        display_gif->drawPixel(x, y, usPalette[*s++]); // color 565
      }
    }
} /* GIFDraw() */

void * GIFOpenFile(const char *fname, int32_t *pSize)
{
  Serial.print("Playing gif: ");
  Serial.println(fname);
  gif_file = SPIFFS.open(fname);
  if (gif_file)
  {
    *pSize = gif_file.size();
    return (void *)&gif_file;
  }
  return NULL;
} /* GIFOpenFile() */

void GIFCloseFile(void *pHandle)
{
  File *f = static_cast<File *>(pHandle);
  if (f != NULL)
     f->close();
} /* GIFCloseFile() */

int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen)
{
    int32_t iBytesRead;
    iBytesRead = iLen;
    File *f = static_cast<File *>(pFile->fHandle);
    // Note: If you read a file all the way to the last byte, seek() stops working
    if ((pFile->iSize - pFile->iPos) < iLen)
       iBytesRead = pFile->iSize - pFile->iPos - 1; // <-- ugly work-around
    if (iBytesRead <= 0)
       return 0;
    iBytesRead = (int32_t)f->read(pBuf, iBytesRead);
    pFile->iPos = f->position();
    return iBytesRead;
} /* GIFReadFile() */

int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition)
{ 
  int i = micros();
  File *f = static_cast<File *>(pFile->fHandle);
  f->seek(iPosition);
  pFile->iPos = (int32_t)f->position();
  i = micros() - i;
//  Serial.printf("Seek time = %d us\n", i);
  return pFile->iPos;
} /* GIFSeekFile() */

void Play_GIF(bool isPlay, char *name, int speed_ms, int duration, bool *stopFlag)
{
  display_gif->clearScreen();
  display_gif->flush();

  int startMillis = millis();

  if (gif.open(name, GIFOpenFile, GIFCloseFile, GIFReadFile, GIFSeekFile, GIFDraw))
  {
    x_offset = (LEDPANEL_WIDTH_GIF - gif.getCanvasWidth())/2;
    if (x_offset < 0) x_offset = 0;
    y_offset = (LEDPANEL_HEIGHT_GIF - gif.getCanvasHeight())/2;
    if (y_offset < 0) y_offset = 0;
    // Serial.printf("Successfully opened GIF; Canvas size = %d x %d\n", gif.getCanvasWidth(), gif.getCanvasHeight());
    // Serial.flush();
    while (millis() - startMillis <= duration && *stopFlag == false)
    {      
      gif.playFrame(true, NULL);
      delay(speed_ms); // adjust this value to control the speed of the GIF
    }
    gif.close();
  }

} /* Play_GIF() */

void Play_Animation(bool isPlay, char *name, int speed_ms, int duration, bool *stopFlag) {
  int startMillis = millis();
  if (isPlay) {
    Play_GIF(isPlay, name, speed_ms, duration, stopFlag);
  }
}