#ifndef SDLDISPLAY_H
#define SDLDISPLAY_H

#include "../Config.h"
#ifdef USE_SDL_DISPLAY
#include "DisplayDriver.h"
#include <SDL2/SDL.h>
#include <cstdint>
#include <stdexcept>

class SDLDisplay : public DisplayDriver {
private:
  static const uint16_t BORDERWIDTH = (Config::LCDWIDTH - 320) / 2;
  static const uint16_t BORDERHEIGHT = (Config::LCDHEIGHT - 200) / 2;
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *texture = nullptr;
  uint16_t *bitmap16;

public:
  SDLDisplay();
  ~SDLDisplay();
  void init() override;
  void drawFrame(uint8_t frameColor) override;
  void drawBitmap(const uint8_t *bitmap, const uint8_t *vicreg) override;
};
#endif

#endif
