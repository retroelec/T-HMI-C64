#include "../Config.h"
#ifdef USE_SDL_DISPLAY
#include "SDLDisplay.h"

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
  renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
  if (!renderer) {
    throw std::runtime_error("SDL_CreateRenderer failed");
  }
  texture = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_RGB565,
                              SDL_TEXTUREACCESS_STREAMING, Config::LCDWIDTH,
                              Config::LCDHEIGHT);
  if (!texture) {
    throw std::runtime_error("SDL_CreateTexture failed");
  }
}

void SDLDisplay::drawBitmap(uint16_t *bitmap) {
  SDL_UpdateTexture(texture, nullptr, bitmap,
                    Config::LCDWIDTH * sizeof(uint16_t));
  SDL_RenderClear(renderer);
  SDL_RenderCopy(renderer, texture, nullptr, nullptr);
  SDL_RenderPresent(renderer);
}

void SDLDisplay::drawFrame(uint16_t frameColor) {}

const uint16_t *SDLDisplay::getC64Colors() const { return c64Colors; }
#endif
