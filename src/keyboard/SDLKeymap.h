#ifndef SDLKEYMAP_H
#define SDLKEYMAP_H

#include "../Config.h"
#ifdef USE_SDL_KEYBOARD
#include <SDL2/SDL.h>
#include <map>
#include <tuple>

using KeySpec =
    std::tuple<SDL_Keycode, bool, bool, bool>; // key, shift, ctrl, commodore
using CodeTriple = std::tuple<uint8_t, uint8_t, uint8_t>;

extern const std::map<KeySpec, CodeTriple> keyMap;

const CodeTriple C64_KEYCODE_BREAK = {0x7f, 0x7f, 0x80};
const CodeTriple C64_KEYCODE_RETURN = {0xfe, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_BACKSPACE = {0xfe, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_SPACE = {0x7f, 0xef, 0x00};
const CodeTriple C64_KEYCODE_0 = {0xef, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_1 = {0x7f, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_2 = {0x7f, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_3 = {0xfd, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_4 = {0xfd, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_5 = {0xfb, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_6 = {0xfb, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_7 = {0xf7, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_8 = {0xf7, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_9 = {0xef, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_0_CTRL = {0xef, 0xf7, 0x02};
const CodeTriple C64_KEYCODE_1_CTRL = {0x7f, 0xfe, 0x02};
const CodeTriple C64_KEYCODE_2_CTRL = {0x7f, 0xf7, 0x02};
const CodeTriple C64_KEYCODE_3_CTRL = {0xfd, 0xfe, 0x02};
const CodeTriple C64_KEYCODE_4_CTRL = {0xfd, 0xf7, 0x02};
const CodeTriple C64_KEYCODE_5_CTRL = {0xfb, 0xfe, 0x02};
const CodeTriple C64_KEYCODE_6_CTRL = {0xfb, 0xf7, 0x02};
const CodeTriple C64_KEYCODE_7_CTRL = {0xf7, 0xfe, 0x02};
const CodeTriple C64_KEYCODE_8_CTRL = {0xf7, 0xf7, 0x02};
const CodeTriple C64_KEYCODE_9_CTRL = {0xef, 0xfe, 0x02};
const CodeTriple C64_KEYCODE_1_COMMODORE = {0x7f, 0xfe, 0x04};
const CodeTriple C64_KEYCODE_2_COMMODORE = {0x7f, 0xf7, 0x04};
const CodeTriple C64_KEYCODE_3_COMMODORE = {0xfd, 0xfe, 0x04};
const CodeTriple C64_KEYCODE_4_COMMODORE = {0xfd, 0xf7, 0x04};
const CodeTriple C64_KEYCODE_5_COMMODORE = {0xfb, 0xfe, 0x04};
const CodeTriple C64_KEYCODE_6_COMMODORE = {0xfb, 0xf7, 0x04};
const CodeTriple C64_KEYCODE_7_COMMODORE = {0xf7, 0xfe, 0x04};
const CodeTriple C64_KEYCODE_8_COMMODORE = {0xf7, 0xf7, 0x04};
const CodeTriple C64_KEYCODE_W = {0xfd, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_A = {0xfd, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_Z = {0xfd, 0xef, 0x00};
const CodeTriple C64_KEYCODE_S = {0xfd, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_E = {0xfd, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_R = {0xfb, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_D = {0xfb, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_C = {0xfb, 0xef, 0x00};
const CodeTriple C64_KEYCODE_F = {0xfb, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_T = {0xfb, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_X = {0xfb, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_Y = {0xf7, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_G = {0xf7, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_B = {0xf7, 0xef, 0x00};
const CodeTriple C64_KEYCODE_H = {0xf7, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_U = {0xf7, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_V = {0xf7, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_I = {0xef, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_J = {0xef, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_M = {0xef, 0xef, 0x00};
const CodeTriple C64_KEYCODE_K = {0xef, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_O = {0xef, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_N = {0xef, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_P = {0xdf, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_L = {0xdf, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_MINUS = {0xdf, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_PERIOD = {0xdf, 0xef, 0x00};
const CodeTriple C64_KEYCODE_COMMA = {0xdf, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_Q = {0x7f, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_DOLLAR = {0xfd, 0xf7, 0x01};
const CodeTriple C64_KEYCODE_LESS = {0xdf, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_RIGHT = {0xfe, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_LEFT = {0xfe, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_DOWN = {0xfe, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_UP = {0xfe, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_F1 = {0xfe, 0xef, 0x00};
const CodeTriple C64_KEYCODE_F3 = {0xfe, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_F5 = {0xfe, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_F7 = {0xfe, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_W_SHIFT = {0xfd, 0xfd, 0x01};
const CodeTriple C64_KEYCODE_A_SHIFT = {0xfd, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_Z_SHIFT = {0xfd, 0xef, 0x01};
const CodeTriple C64_KEYCODE_S_SHIFT = {0xfd, 0xdf, 0x01};
const CodeTriple C64_KEYCODE_E_SHIFT = {0xfd, 0xbf, 0x01};
const CodeTriple C64_KEYCODE_R_SHIFT = {0xfb, 0xfd, 0x01};
const CodeTriple C64_KEYCODE_D_SHIFT = {0xfb, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_C_SHIFT = {0xfb, 0xef, 0x01};
const CodeTriple C64_KEYCODE_F_SHIFT = {0xfb, 0xdf, 0x01};
const CodeTriple C64_KEYCODE_T_SHIFT = {0xfb, 0xbf, 0x01};
const CodeTriple C64_KEYCODE_X_SHIFT = {0xfb, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_Y_SHIFT = {0xf7, 0xfd, 0x01};
const CodeTriple C64_KEYCODE_G_SHIFT = {0xf7, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_B_SHIFT = {0xf7, 0xef, 0x01};
const CodeTriple C64_KEYCODE_H_SHIFT = {0xf7, 0xdf, 0x01};
const CodeTriple C64_KEYCODE_U_SHIFT = {0xf7, 0xbf, 0x01};
const CodeTriple C64_KEYCODE_V_SHIFT = {0xf7, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_I_SHIFT = {0xef, 0xfd, 0x01};
const CodeTriple C64_KEYCODE_J_SHIFT = {0xef, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_M_SHIFT = {0xef, 0xef, 0x01};
const CodeTriple C64_KEYCODE_K_SHIFT = {0xef, 0xdf, 0x01};
const CodeTriple C64_KEYCODE_O_SHIFT = {0xef, 0xbf, 0x01};
const CodeTriple C64_KEYCODE_N_SHIFT = {0xef, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_P_SHIFT = {0xdf, 0xfd, 0x01};
const CodeTriple C64_KEYCODE_L_SHIFT = {0xdf, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_Q_SHIFT = {0x7f, 0xbf, 0x01};
const CodeTriple C64_KEYCODE_QUOTEDBL = {0x7f, 0xf7, 0x01};
const CodeTriple C64_KEYCODE_PERCENT = {0xfb, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_AMPERSAND = {0xfb, 0xf7, 0x01};
const CodeTriple C64_KEYCODE_LEFTPAREN = {0xf7, 0xf7, 0x01};
const CodeTriple C64_KEYCODE_RIGHTPAREN = {0xef, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_GREATER = {0xdf, 0xef, 0x01};
const CodeTriple C64_KEYCODE_QUESTION = {0xbf, 0x7f, 0x01};
const CodeTriple C64_KEYCODE_PLUS = {0xdf, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_ASTERISK = {0xbf, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_SLASH = {0xbf, 0x7f, 0x00};
const CodeTriple C64_KEYCODE_EQUALS = {0xbf, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_COLON = {0xdf, 0xdf, 0x00};
const CodeTriple C64_KEYCODE_SEMICOLON = {0xbf, 0xfb, 0x00};
const CodeTriple C64_KEYCODE_EXCLAIM = {0x7f, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_AT = {0xdf, 0xbf, 0x00};
const CodeTriple C64_KEYCODE_HASH = {0xfd, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_QUOTE = {0xf7, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_HOME = {0xbf, 0xf7, 0x00};
const CodeTriple C64_KEYCODE_CLR = {0xbf, 0xf7, 0x01};
const CodeTriple C64_KEYCODE_DEL = {0xfe, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_INS = {0xfe, 0xfe, 0x01};
const CodeTriple C64_KEYCODE_LEFTBRACKET = {0xdf, 0xdf, 0x01};
const CodeTriple C64_KEYCODE_RIGHTBRACKET = {0xbf, 0xfb, 0x01};
const CodeTriple C64_KEYCODE_LEFTARROW = {0x7f, 0xfd, 0x00};
const CodeTriple C64_KEYCODE_POUND = {0xbf, 0xfe, 0x00};
const CodeTriple C64_KEYCODE_UPARROW = {0xbf, 0xbf, 0x00};
#endif

#endif // SDLKEYMAP_H
