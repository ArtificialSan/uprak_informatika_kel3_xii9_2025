#ifndef GIFPLAYER_H
#define GIFPLAYER_H

#include <Arduino.h>
#include <AnimatedGIF.h>
#include <ESP32-HUB75-MatrixPanel-I2S-DMA.h>
#include "SPIFFS.h"
#include <GifDecoder.h>

void gif_player_init(MatrixPanel_I2S_DMA *display_placeholder, int width, int height);
void GIFDraw(GIFDRAW *pDraw);
void * GIFOpenFile(const char *fname, int32_t *pSize);
int32_t GIFReadFile(GIFFILE *pFile, uint8_t *pBuf, int32_t iLen);
int32_t GIFSeekFile(GIFFILE *pFile, int32_t iPosition);
void Play_GIF(bool isPlay, char *name, int speed_ms, int duration, bool *stopFlag);
void Play_Animation(bool isPlay, char *name, int speed_ms, int duration, bool *stopFlag);

#endif