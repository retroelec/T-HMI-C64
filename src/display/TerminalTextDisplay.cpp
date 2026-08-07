/*
 Copyright (C) 2024-2026 retroelec <retroelec42@gmail.com>

 This program is free software; you can redistribute it and/or modify it
 under the terms of the GNU General Public License as published by the
 Free Software Foundation; either version 3 of the License, or (at your
 option) any later version.

 This program is distributed in the hope that it will be useful, but
 WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY
 or FITNESS FOR A PARTICULAR PURPOSE. See the GNU General Public License
 for more details.

 For the complete text of the GNU General Public License see
 http://www.gnu.org/licenses/.
*/
#include "../Config.h"
#ifdef USE_TERMINALTEXT_DISPLAY

#include "TerminalTextDisplay.h"
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <unistd.h>

static struct termios savedTermios;
static bool termInitialized = false;

static void cleanupTerminal() {
  if (termInitialized) {
    const char *restore = "\x1b[?25h\x1b[?1049l";
    write(STDOUT_FILENO, restore, strlen(restore));
    tcsetattr(STDOUT_FILENO, TCSAFLUSH, &savedTermios);
  }
}

TerminalTextDisplay::TerminalTextDisplay()
    : frameColor(0), prevScreenMemStart(0xFFFF), prevBgColor(0xFF) {
  memset(prevScreen, 0, sizeof(prevScreen));
}

TerminalTextDisplay::~TerminalTextDisplay() { cleanupTerminal(); }

void TerminalTextDisplay::setRawMode() {
  struct termios raw;
  tcgetattr(STDOUT_FILENO, &savedTermios);
  raw = savedTermios;
  raw.c_iflag &= ~(BRKINT | ICRNL | INPCK | ISTRIP | IXON);
  raw.c_oflag &= ~(OPOST);
  raw.c_cflag |= (CS8);
  raw.c_lflag &= ~(ECHO | ICANON | IEXTEN | ISIG);
  raw.c_cc[VMIN] = 1;
  raw.c_cc[VTIME] = 0;
  tcsetattr(STDOUT_FILENO, TCSAFLUSH, &raw);
}

void TerminalTextDisplay::restoreTerminal() { cleanupTerminal(); }

void TerminalTextDisplay::init() {
  setRawMode();
  atexit(cleanupTerminal);
  const char *initseq = "\x1b[?1049h\x1b[?25l\x1b[2J";
  write(STDOUT_FILENO, initseq, strlen(initseq));
  termInitialized = true;
}

void TerminalTextDisplay::drawFrame(uint8_t fcolor) { frameColor = fcolor; }

// color buffer helpers: write ANSI SGR to buf, return new buf position
static char *writeAnsiSgr(char *buf, uint8_t c64color, bool bg) {
  // C64 RGB565 values from DisplayDriver.h
  static const uint16_t c64Colors[16] = {
      0x0000, 0xffff, 0x8000, 0xa7fc, 0xc218, 0x064a, 0x0014, 0xe74e,
      0xd42a, 0x6200, 0xfbae, 0x3186, 0x73ae, 0xa7ec, 0x043f, 0xb5d6};
  uint16_t rgb565 = c64Colors[c64color];
  uint8_t r = ((rgb565 >> 11) & 0x1F) * 255 / 31;
  uint8_t g = ((rgb565 >> 5) & 0x3F) * 255 / 63;
  uint8_t b = (rgb565 & 0x1F) * 255 / 31;
  if (bg) {
    buf += sprintf(buf, "\x1b[48;2;%d;%d;%dm", r, g, b);
  } else {
    buf += sprintf(buf, "\x1b[38;2;%d;%d;%dm", r, g, b);
  }
  return buf;
}

uint32_t TerminalTextDisplay::petsciiToUnicode(uint8_t c) {
  // C64 Video-RAM Screen-Code to Unicode mapping (default uppercase/graphics
  // mode)
  static const uint32_t table[256] = {
      0x0040, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046, 0x0047, 0x0048,
      0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F, 0x0050, 0x0051,
      0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058, 0x0059, 0x005A,
      0x005B, 0x00A3, 0x005D, 0x2191, 0x2190, 0x0020, 0x0021, 0x0022, 0x0023,
      0x0024, 0x0025, 0x0026, 0x0027, 0x0028, 0x0029, 0x002A, 0x002B, 0x002C,
      0x002D, 0x002E, 0x002F, 0x0030, 0x0031, 0x0032, 0x0033, 0x0034, 0x0035,
      0x0036, 0x0037, 0x0038, 0x0039, 0x003A, 0x003B, 0x003C, 0x003D, 0x003E,
      0x003F, 0x258C, 0x2580, 0x2584, 0x2588, 0x2591, 0x2592, 0x2593, 0x25A0,
      0x25CB, 0x25CF, 0x2665, 0x2660, 0x2663, 0x2666, 0x266A, 0x25D8, 0x25CA,
      0x2603, 0x25B6, 0x25C0, 0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518,
      0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x2591,
      0x2592, 0x2593, 0x25A0, 0x25CB, 0x25CF, 0x2665, 0x2660, 0x2663, 0x2666,
      0x266A, 0x25D8, 0x25CA, 0x2603, 0x25B6, 0x25C0, 0x2500, 0x2502, 0x250C,
      0x2510, 0x2514, 0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580,
      0x2584, 0x2588, 0x2591, 0x2592, 0x2593, 0x25A0, 0x25CB, 0x25CF, 0x2665,
      0x2660, 0x2663, 0x2666, 0x266A, 0x25D8, 0x25CA, 0x2603, 0x25B6, 0x25C0,
      0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518, 0x251C, 0x2524, 0x252C,
      0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x2591, 0x2592, 0x2593, 0x25A0,
      0x25CB, 0x25CF, 0x2665, 0x2660, 0x2663, 0x2666, 0x266A, 0x25D8, 0x25CA,
      0x2603, 0x25B6, 0x25C0, 0x2500, 0x2502, 0x250C, 0x2510, 0x2514, 0x2518,
      0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580, 0x2584, 0x2588, 0x2591,
      0x2592, 0x2593, 0x25A0, 0x0041, 0x0042, 0x0043, 0x0044, 0x0045, 0x0046,
      0x0047, 0x0048, 0x0049, 0x004A, 0x004B, 0x004C, 0x004D, 0x004E, 0x004F,
      0x0050, 0x0051, 0x0052, 0x0053, 0x0054, 0x0055, 0x0056, 0x0057, 0x0058,
      0x0059, 0x005A, 0x25CB, 0x25CF, 0x2665, 0x2660, 0x2663, 0x2666, 0x266A,
      0x25D8, 0x25CA, 0x2603, 0x25B6, 0x25C0, 0x2500, 0x2502, 0x250C, 0x2510,
      0x2514, 0x2518, 0x251C, 0x2524, 0x252C, 0x2534, 0x253C, 0x2580, 0x2584,
      0x2588, 0x2591, 0x2592, 0x2593, 0x25A0, 0x25CB, 0x25CF, 0x2665, 0x2660,
      0x2663, 0x2666, 0x266A, 0x25D8,
  };
  return table[c];
}

// encode a Unicode code point to UTF-8, return bytes written
static int encodeUtf8(uint32_t unicode, char *out) {
  if (unicode < 0x80) {
    out[0] = (char)unicode;
    return 1;
  } else if (unicode < 0x800) {
    out[0] = 0xC0 | (unicode >> 6);
    out[1] = 0x80 | (unicode & 0x3F);
    return 2;
  } else {
    out[0] = 0xE0 | (unicode >> 12);
    out[1] = 0x80 | ((unicode >> 6) & 0x3F);
    out[2] = 0x80 | (unicode & 0x3F);
    return 3;
  }
}

void TerminalTextDisplay::renderScreen(const uint8_t *ram,
                                       const uint8_t *vicreg) {
  uint16_t screenmemstart = ((vicreg[0x18] >> 4) & 7) * 1024;
  uint8_t bgcolor = vicreg[0x21] & 15;

  char buf[7000];
  char *p = buf;
  char *end = buf + sizeof(buf) - 1;

  // set foreground + background colors
  p = writeAnsiSgr(p, frameColor, false);
  p = writeAnsiSgr(p, bgcolor, true);

  for (int y = 0; y < 25; y++) {
    int n = snprintf(p, end - p, "\x1b[%d;1H", y + 1);
    if (n > 0)
      p += n;

    // left border (5 chars, frame color)
    p = writeAnsiSgr(p, frameColor, true);
    for (int i = 0; i < 5; i++) {
      *p++ = ' ';
    }

    // C64 text area (40 chars, vicreg[0x21] background)
    p = writeAnsiSgr(p, bgcolor, true);
    for (int x = 0; x < 40; x++) {
      uint8_t ch = ram[screenmemstart + y * 40 + x];
      uint32_t unicode = petsciiToUnicode(ch);
      int bytes = encodeUtf8(unicode, p);
      p += bytes;
      if (p >= end)
        break;
    }

    // right border (5 chars, frame color)
    p = writeAnsiSgr(p, frameColor, true);
    for (int i = 0; i < 5; i++) {
      *p++ = ' ';
    }

    if (p >= end)
      break;
  }

  *p = '\0';
  write(STDOUT_FILENO, buf, p - buf);
}

void TerminalTextDisplay::drawBitmap(const uint8_t *bitmap,
                                     const uint8_t *vicreg,
                                     const uint8_t *ram) {
  (void)bitmap;

  uint16_t screenmemstart = ((vicreg[0x18] >> 4) & 7) * 1024;
  uint8_t bgcolor = vicreg[0x21] & 15;

  // only redraw if something changed
  if (screenmemstart == prevScreenMemStart && bgcolor == prevBgColor &&
      memcmp(prevScreen, ram + screenmemstart, 1000) == 0) {
    return;
  }

  prevScreenMemStart = screenmemstart;
  prevBgColor = bgcolor;

  // save current screen content for diff
  memcpy(prevScreen, ram + screenmemstart, 1000);

  renderScreen(ram, vicreg);
}

#endif