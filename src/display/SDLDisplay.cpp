#include "../Config.h"
#ifdef USE_SDL_DISPLAY
#include "SDLDisplay.h"
#include <SDL2/SDL.h>

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

struct Overlay {
  char digit1 = 0;
  char digit2 = 0;
  uint32_t startTicks = 0;
  uint16_t duration = 2000; // in ms
  bool active = false;
};

Overlay overlay;

void SDLDisplay::dispOverlayInfo(char digit1, char digit2) {
  overlay.digit1 = digit1;
  overlay.digit2 = digit2;
  overlay.startTicks = SDL_GetTicks();
  overlay.active = true;
}

static const uint8_t seg[10] = {
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

void drawDigit(SDL_Renderer *renderer, int digit, int x, int y, int size) {
  if (digit < 0 || digit > 9) {
    return;
  }
  int thickness = size / 3;
  int length = size;
  SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
  SDL_Rect a = {x + thickness, y, length, thickness};
  SDL_Rect b = {x + length + thickness, y + thickness, thickness, length};
  SDL_Rect c = {x + length + thickness, y + length + 2 * thickness, thickness,
                length};
  SDL_Rect d = {x + thickness, y + 2 * length + 2 * thickness, length,
                thickness};
  SDL_Rect e = {x, y + length + 2 * thickness, thickness, length};
  SDL_Rect f = {x, y + thickness, thickness, length};
  SDL_Rect g = {x + thickness, y + length + thickness, length, thickness};
  if (seg[digit] & (1 << 0))
    SDL_RenderFillRect(renderer, &a);
  if (seg[digit] & (1 << 1))
    SDL_RenderFillRect(renderer, &b);
  if (seg[digit] & (1 << 2))
    SDL_RenderFillRect(renderer, &c);
  if (seg[digit] & (1 << 3))
    SDL_RenderFillRect(renderer, &d);
  if (seg[digit] & (1 << 4))
    SDL_RenderFillRect(renderer, &e);
  if (seg[digit] & (1 << 5))
    SDL_RenderFillRect(renderer, &f);
  if (seg[digit] & (1 << 6))
    SDL_RenderFillRect(renderer, &g);
}

void drawDigits(SDL_Renderer *r, int x, int y, int size) {
  drawDigit(r, overlay.digit1 - '0', x, y, size);
  x += size + 8;
  drawDigit(r, overlay.digit2 - '0', x, y, size);
}

void renderOverlay(SDL_Renderer *renderer) {
  if (!overlay.active)
    return;
  uint32_t now = SDL_GetTicks();
  if (now > overlay.startTicks + overlay.duration) {
    overlay.active = false;
    return;
  }
  drawDigits(renderer, 320, 200, 8);
}

void SDLDisplay::drawBitmap(uint16_t *bitmap) {
  SDL_UpdateTexture(texture, nullptr, bitmap, 320 * sizeof(uint16_t));
  SDL_Rect dst{BORDERWIDTH, BORDERHEIGHT, 320, 200};
  SDL_RenderCopy(renderer, texture, nullptr, &dst);
  renderOverlay(renderer);
  SDL_RenderPresent(renderer);
}

const uint16_t *SDLDisplay::getC64Colors() const { return c64Colors; }
#endif
