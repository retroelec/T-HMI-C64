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
#ifndef C64PASTEMAPPING_H
#define C64PASTEMAPPING_H

#include "CodeTripleDef.h"
#include <cstdint>
#include <optional>

/**
 * @brief Maps ASCII characters (0x00-0x7F) to C64 matrix codes.
 *
 * Letters are always mapped to C64 uppercase characters
 * (as after powering on the C64). This means:
 * - Linux `a` and `A` → C64 key A without shift → yields `A`
 * - Linux `b` and `B` → C64 key B without shift → yields `B`
 * - etc.
 *
 * Digits and punctuation are mapped to the corresponding C64 matrix
 * triple (with or without shift modifier as needed).
 */
inline std::optional<CodeTripleS> getC64CodeForChar(uint32_t codepoint) {
  if (codepoint > 0x7f) {
    return std::nullopt;
  }
  switch (static_cast<char>(codepoint)) {
  case 'a':
  case 'A':
    return CodeTripleS{0xfd, 0xfb, 0x00};
  case 'b':
  case 'B':
    return CodeTripleS{0xf7, 0xef, 0x00};
  case 'c':
  case 'C':
    return CodeTripleS{0xfb, 0xef, 0x00};
  case 'd':
  case 'D':
    return CodeTripleS{0xfb, 0xfb, 0x00};
  case 'e':
  case 'E':
    return CodeTripleS{0xfd, 0xbf, 0x00};
  case 'f':
  case 'F':
    return CodeTripleS{0xfb, 0xdf, 0x00};
  case 'g':
  case 'G':
    return CodeTripleS{0xf7, 0xfb, 0x00};
  case 'h':
  case 'H':
    return CodeTripleS{0xf7, 0xdf, 0x00};
  case 'i':
  case 'I':
    return CodeTripleS{0xef, 0xfd, 0x00};
  case 'j':
  case 'J':
    return CodeTripleS{0xef, 0xfb, 0x00};
  case 'k':
  case 'K':
    return CodeTripleS{0xef, 0xdf, 0x00};
  case 'l':
  case 'L':
    return CodeTripleS{0xdf, 0xfb, 0x00};
  case 'm':
  case 'M':
    return CodeTripleS{0xef, 0xef, 0x00};
  case 'n':
  case 'N':
    return CodeTripleS{0xef, 0x7f, 0x00};
  case 'o':
  case 'O':
    return CodeTripleS{0xef, 0xbf, 0x00};
  case 'p':
  case 'P':
    return CodeTripleS{0xdf, 0xfd, 0x00};
  case 'q':
  case 'Q':
    return CodeTripleS{0x7f, 0xbf, 0x00};
  case 'r':
  case 'R':
    return CodeTripleS{0xfb, 0xfd, 0x00};
  case 's':
  case 'S':
    return CodeTripleS{0xfd, 0xdf, 0x00};
  case 't':
  case 'T':
    return CodeTripleS{0xfb, 0xbf, 0x00};
  case 'u':
  case 'U':
    return CodeTripleS{0xf7, 0xbf, 0x00};
  case 'v':
  case 'V':
    return CodeTripleS{0xf7, 0x7f, 0x00};
  case 'w':
  case 'W':
    return CodeTripleS{0xfd, 0xfd, 0x00};
  case 'x':
  case 'X':
    return CodeTripleS{0xfb, 0x7f, 0x00};
  case 'y':
  case 'Y':
    return CodeTripleS{0xf7, 0xfd, 0x00};
  case 'z':
  case 'Z':
    return CodeTripleS{0xfd, 0xef, 0x00};
  case '0':
    return CodeTripleS{0xef, 0xf7, 0x00};
  case '1':
    return CodeTripleS{0x7f, 0xfe, 0x00};
  case '2':
    return CodeTripleS{0x7f, 0xf7, 0x00};
  case '3':
    return CodeTripleS{0xfd, 0xfe, 0x00};
  case '4':
    return CodeTripleS{0xfd, 0xf7, 0x00};
  case '5':
    return CodeTripleS{0xfb, 0xfe, 0x00};
  case '6':
    return CodeTripleS{0xfb, 0xf7, 0x00};
  case '7':
    return CodeTripleS{0xf7, 0xfe, 0x00};
  case '8':
    return CodeTripleS{0xf7, 0xf7, 0x00};
  case '9':
    return CodeTripleS{0xef, 0xfe, 0x00};
  case ' ':
    return CodeTripleS{0x7f, 0xef, 0x00};
  case '!':
    return CodeTripleS{0x7f, 0xfe, 0x01}; // EXCLAIM = 1+Shift
  case '"':
    return CodeTripleS{0x7f, 0xf7, 0x01}; // QUOTEDBL
  case '#':
    return CodeTripleS{0xfd, 0xfe, 0x01}; // HASH
  case '$':
    return CodeTripleS{0xfd, 0xf7, 0x01}; // DOLLAR
  case '%':
    return CodeTripleS{0xfb, 0xfe, 0x01}; // PERCENT
  case '&':
    return CodeTripleS{0xfb, 0xf7, 0x01}; // AMPERSAND
  case '\'':
    return CodeTripleS{0xf7, 0xfe, 0x01}; // QUOTE
  case '(':
    return CodeTripleS{0xf7, 0xf7, 0x01}; // LEFTPAREN
  case ')':
    return CodeTripleS{0xef, 0xfe, 0x01}; // RIGHTPAREN
  case '*':
    return CodeTripleS{0xbf, 0xfd, 0x00}; // ASTERISK
  case '+':
    return CodeTripleS{0xdf, 0xfe, 0x00}; // PLUS
  case ',':
    return CodeTripleS{0xdf, 0x7f, 0x00}; // COMMA
  case '-':
    return CodeTripleS{0xdf, 0xf7, 0x00}; // MINUS
  case '.':
    return CodeTripleS{0xdf, 0xef, 0x00}; // PERIOD
  case '/':
    return CodeTripleS{0xbf, 0x7f, 0x00}; // SLASH
  case ':':
    return CodeTripleS{0xdf, 0xdf, 0x00}; // COLON
  case ';':
    return CodeTripleS{0xbf, 0xfb, 0x00}; // SEMICOLON
  case '<':
    return CodeTripleS{0xdf, 0x7f, 0x01}; // LESS = ,+Shift
  case '=':
    return CodeTripleS{0xbf, 0xdf, 0x00}; // EQUALS
  case '>':
    return CodeTripleS{0xdf, 0xef, 0x01}; // GREATER = .+Shift
  case '?':
    return CodeTripleS{0xbf, 0x7f, 0x01}; // QUESTION = /+Shift
  case '@':
    return CodeTripleS{0xdf, 0xbf, 0x00}; // AT
  case '[':
    return CodeTripleS{0xdf, 0xdf, 0x01}; // LEFTBRACKET = :+Shift
  case '\\':
    return CodeTripleS{0xbf, 0xfe, 0x00}; // POUND
  case ']':
    return CodeTripleS{0xbf, 0xfb, 0x01}; // RIGHTBRACKET = ;+Shift
  case '^':
    return CodeTripleS{0xbf, 0xbf, 0x00}; // UPARROW
  case '_':
    return CodeTripleS{0x7f, 0xfd, 0x00}; // LEFTARROW
  case '`':
    return std::nullopt; // not available on C64
  case '{':
    return std::nullopt; // not available on C64
  case '|':
    return std::nullopt; // not available on C64
  case '}':
    return std::nullopt; // not available on C64
  case '~':
    return std::nullopt; // not available on C64
    // control character
  case '\n':
  case '\r':
    return CodeTripleS{0xfe, 0xfd, 0x00}; // RETURN
  case '\b':
  case '\x7f':
    return CodeTripleS{0xfe, 0xfe, 0x00}; // BACKSPACE/DEL
  case '\t':
    return CodeTripleS{0x7f, 0xef, 0x00}; // TAB → SPACE
  default:
    return std::nullopt;
  }
}

/**
 * @brief Decodes one UTF-8 character and returns the Unicode codepoint.
 *
 * Advances the pointer `text` past the decoded character.
 * On invalid bytes, the pointer is advanced by 1 and 0 is returned.
 *
 * @param[in,out] text Pointer into a UTF-8 string
 * @return Unicode codepoint (0 on error)
 */
inline uint32_t decodeUtf8Char(const char *&text) {
  uint8_t c = static_cast<uint8_t>(*text);
  if (c < 0x80) {
    text++;
    return c;
  }
  if ((c & 0xe0) == 0xc0) {
    // 2-Byte-Sequenz: 110xxxxx 10xxxxxx
    if ((static_cast<uint8_t>(text[1]) & 0xc0) != 0x80) {
      text++;
      return 0;
    }
    uint32_t cp = ((c & 0x1f) << 6) | (static_cast<uint8_t>(text[1]) & 0x3f);
    text += 2;
    return cp;
  }
  if ((c & 0xf0) == 0xe0) {
    // 3-Byte-Sequenz: 1110xxxx 10xxxxxx 10xxxxxx
    if ((static_cast<uint8_t>(text[1]) & 0xc0) != 0x80 ||
        (static_cast<uint8_t>(text[2]) & 0xc0) != 0x80) {
      text++;
      return 0;
    }
    uint32_t cp = ((c & 0x0f) << 12) |
                  ((static_cast<uint8_t>(text[1]) & 0x3f) << 6) |
                  (static_cast<uint8_t>(text[2]) & 0x3f);
    text += 3;
    return cp;
  }
  if ((c & 0xf8) == 0xf0) {
    // 4-Byte-Sequenz: 11110xxx 10xxxxxx 10xxxxxx 10xxxxxx
    if ((static_cast<uint8_t>(text[1]) & 0xc0) != 0x80 ||
        (static_cast<uint8_t>(text[2]) & 0xc0) != 0x80 ||
        (static_cast<uint8_t>(text[3]) & 0xc0) != 0x80) {
      text++;
      return 0;
    }
    uint32_t cp = ((c & 0x07) << 18) |
                  ((static_cast<uint8_t>(text[1]) & 0x3f) << 12) |
                  ((static_cast<uint8_t>(text[2]) & 0x3f) << 6) |
                  (static_cast<uint8_t>(text[3]) & 0x3f);
    text += 4;
    return cp;
  }
  // invalid byte
  text++;
  return 0;
}

#endif // C64PASTEMAPPING_H
