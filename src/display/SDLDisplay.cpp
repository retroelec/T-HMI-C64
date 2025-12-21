#include "../Config.h"
#ifdef USE_SDL_DISPLAY
#include "../roms/charset.h"
#include "SDLDisplay.h"
#include <SDL2/SDL.h>

void drawChar(SDL_Renderer *ren, uint16_t c, uint16_t x, uint16_t y,
              uint8_t charpixsize) {
  const uint8_t *glyph = &charset_rom[c * 8];
  SDL_SetRenderDrawColor(ren, 200, 200, 200, 255);
  for (uint8_t row = 0; row < 8; row++) {
    uint8_t bits = glyph[row];
    for (uint8_t col = 0; col < 8; col++) {
      if (bits & (1 << (7 - col))) {
        SDL_Rect pixel = {x + col * charpixsize, y + row * charpixsize,
                          charpixsize, charpixsize};
        SDL_RenderFillRect(ren, &pixel);
      }
    }
  }
}

SDLDisplay::SDLDisplay() {}

SDLDisplay::~SDLDisplay() {
  if (texture) {
    SDL_DestroyTexture(texture);
  }
  if (renderer) {
    SDL_DestroyRenderer(renderer);
  }
  if (window) {
    SDL_DestroyWindow(window);
  }
  SDL_Quit();
}

static void setWindowIcon(SDL_Window *window) {
  const char *iconPath = "thmic64.bmp";
  SDL_Surface *icon = SDL_LoadBMP(iconPath);
  if (!icon) {
    SDL_Log("Icon konnte nicht geladen werden (%s): %s", iconPath,
            SDL_GetError());
    return;
  }
  SDL_SetWindowIcon(window, icon);
  SDL_FreeSurface(icon);
}

void SDLDisplay::init() {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) {
    throw std::runtime_error("SDL_Init failed");
  }
  window = SDL_CreateWindow(
      "T-HMI-C64 Emulator", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
      Config::LCDWIDTH * Config::LCDSCALE, Config::LCDHEIGHT * Config::LCDSCALE,
      SDL_WINDOW_SHOWN);
  if (!window) {
    throw std::runtime_error("SDL_CreateWindow failed");
  }
  setWindowIcon(window);
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    throw std::runtime_error("SDL_CreateRenderer failed");
  }
  SDL_RenderSetLogicalSize(renderer, Config::LCDWIDTH, Config::LCDHEIGHT);
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565,
                              SDL_TEXTUREACCESS_STREAMING, 320, 200);
  if (!texture) {
    throw std::runtime_error("SDL_CreateTexture failed");
  }
}

static inline void setDrawColor565(SDL_Renderer *r, uint16_t c) {
  uint8_t R = ((c >> 11) & 0x1F) * 255 / 31;
  uint8_t G = ((c >> 5) & 0x3F) * 255 / 63;
  uint8_t B = (c & 0x1F) * 255 / 31;
  SDL_SetRenderDrawColor(r, R, G, B, 255);
}

void SDLDisplay::drawFrame(uint16_t frameColor) {
  setDrawColor565(renderer, frameColor);
  SDL_Rect top{0, 0, Config::LCDWIDTH, BORDERHEIGHT};
  SDL_RenderFillRect(renderer, &top);
  SDL_Rect bottom{0, BORDERHEIGHT + 200, Config::LCDWIDTH, BORDERHEIGHT};
  SDL_RenderFillRect(renderer, &bottom);
  SDL_Rect left{0, BORDERHEIGHT, BORDERWIDTH, 200};
  SDL_RenderFillRect(renderer, &left);
  SDL_Rect right{BORDERWIDTH + 320, BORDERHEIGHT, BORDERWIDTH, 200};
  SDL_RenderFillRect(renderer, &right);
}

void SDLDisplay::drawBitmap(uint16_t *bitmap) {
  SDL_UpdateTexture(texture, nullptr, bitmap, 320 * sizeof(uint16_t));
  SDL_Rect dst{BORDERWIDTH, BORDERHEIGHT, 320, 200};
  SDL_RenderCopy(renderer, texture, nullptr, &dst);
  SDL_RenderPresent(renderer);
}

const uint16_t *SDLDisplay::getC64Colors() const { return c64Colors; }
#endif
