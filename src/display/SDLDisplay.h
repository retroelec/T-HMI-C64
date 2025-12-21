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
  static constexpr uint16_t c64Colors[16] = {
      0x0000, 0xffff, 0x8000, 0xa7fc, 0xc218, 0x064a, 0x0014, 0xe74e,
      0xd42a, 0x6200, 0xfbae, 0x3186, 0x73ae, 0xa7ec, 0x043f, 0xb5d6};
  SDL_Window *window = nullptr;
  SDL_Renderer *renderer = nullptr;
  SDL_Texture *texture = nullptr;

public:
  SDLDisplay();
  ~SDLDisplay();
  void init() override;
  void drawFrame(uint16_t frameColor) override;
  void drawBitmap(uint16_t *bitmap) override;
  const uint16_t *getC64Colors() const override;
};
#endif

#endif
