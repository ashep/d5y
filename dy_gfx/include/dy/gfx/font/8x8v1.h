/**
 * Basic 8x8 font, CP-1251.
 */
#pragma once
#include <stdio.h>
#include "dy/gfx/text.h"

static const uint8_t data[] =
    {
        1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 32 (whitespace)
        1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00, 0x80, 0x00,  // ! - 33
        5, 0xd8, 0x48, 0x90, 0x00, 0x00, 0x00, 0x00, 0x00,  // " - 34
        5, 0x00, 0x50, 0xf8, 0x50, 0x50, 0xf8, 0x50, 0x00,  // # - 35
        5, 0x20, 0x78, 0xA0, 0x70, 0x28, 0xF0, 0x20, 0x00,  // $ - 36
        7, 0x42, 0xA4, 0x48, 0x10, 0x24, 0x4A, 0x84, 0x00,  // % - 37
        5, 0x60, 0x90, 0x90, 0x60, 0xa8, 0x90, 0x98, 0x68,  // & - 38
        2, 0xc0, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00,  // ' - 39
        3, 0x20, 0x40, 0x80, 0x80, 0x80, 0x40, 0x20, 0x00,  // ( - 40
        3, 0x80, 0x40, 0x20, 0x20, 0x20, 0x40, 0x80, 0x00,  // ) - 41
        3, 0x00, 0x00, 0x00, 0xA0, 0x40, 0xA0, 0x00, 0x00,  // * - 42
        5, 0x00, 0x00, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x00,  // + - 43
        2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x40,  // , - 44
        4, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00, 0x00, 0x00,  // - - 45
        1, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x80, 0x00,  // . - 46
        6, 0x00, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x00,  // / - 47
        5, 0x70, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x70, 0x00,  // 0 - 48
        5, 0x20, 0x60, 0xA0, 0x20, 0x20, 0x20, 0xF8, 0x00,  // 1 - 49
        5, 0x70, 0x88, 0x08, 0x30, 0x40, 0x80, 0xF8, 0x00,  // 2 - 50
        5, 0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70, 0x00,  // 3 - 51
        5, 0x18, 0x28, 0x48, 0x88, 0xF8, 0x08, 0x08, 0x00,  // 4 - 52
        5, 0xF8, 0x80, 0xF0, 0x08, 0x08, 0x08, 0xF0, 0x00,  // 5 - 53
        5, 0x70, 0x88, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00,  // 6 - 54
        5, 0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0x80, 0x00,  // 7 - 55
        5, 0x70, 0x88, 0x88, 0x70, 0x88, 0x88, 0x70, 0x00,  // 8 - 56
        5, 0x70, 0x88, 0x88, 0x78, 0x08, 0x88, 0x70, 0x00,  // 9 - 57
        1, 0x00, 0x00, 0x80, 0x00, 0x00, 0x80, 0x00, 0x00,  // : - 58
        2, 0x00, 0x00, 0x40, 0x00, 0x00, 0x40, 0x80, 0x00,  // ; - 59
        4, 0x00, 0x10, 0x20, 0x40, 0x80, 0x40, 0x20, 0x10,  // < - 60
        4, 0x00, 0x00, 0x00, 0xf0, 0x00, 0xf0, 0x00, 0x00,  // = - 61
        4, 0x00, 0x80, 0x40, 0x20, 0x10, 0x20, 0x40, 0x80,  // > - 62
        5, 0x70, 0x88, 0x08, 0x10, 0x20, 0x00, 0x20, 0x00,  // ? - 63
        4, 0x60, 0x90, 0xb0, 0xb0, 0xb0, 0x80, 0x70, 0x00,  // @ - 64
        5, 0x38, 0x48, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,  // A - 65
        5, 0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00,  // B - 66
        5, 0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00,  // C - 67
        5, 0xF0, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF0, 0x00,  // D - 68
        5, 0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF8, 0x00,  // E - 69
        5, 0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0x80, 0x00,  // F - 70
        5, 0x78, 0x80, 0x80, 0x80, 0x98, 0x88, 0x78, 0x00,  // G - 71
        5, 0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88, 0x00,  // H - 72
        3, 0xe0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xe0, 0x00,  // I - 73
        4, 0x30, 0x10, 0x10, 0x10, 0x10, 0x90, 0x60, 0x00,  // J - 74
        5, 0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00,  // K - 75
        4, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0xF0, 0x00,  // L - 76
        5, 0x88, 0xD8, 0xA8, 0x88, 0x88, 0x88, 0x88, 0x00,  // M - 77
        5, 0x88, 0x88, 0xC8, 0xA8, 0x98, 0x88, 0x88, 0x00,  // N - 78
        5, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,  // O - 79
        5, 0xf0, 0x88, 0x88, 0x88, 0xf0, 0x80, 0x80, 0x00,  // P - 80
        5, 0x70, 0x88, 0x88, 0x88, 0xA8, 0x90, 0x68, 0x00,  // Q - 81
        5, 0xF0, 0x88, 0x88, 0x88, 0xF0, 0x90, 0x88, 0x00,  // R - 82
        5, 0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00,  // S - 83
        5, 0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,  // T - 84
        5, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,  // U - 85
        5, 0x88, 0x88, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00,  // V - 86
        5, 0x88, 0x88, 0x88, 0xa8, 0xa8, 0xd8, 0x88, 0x00,  // W - 87
        5, 0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00,  // X - 88
        5, 0x88, 0x88, 0x88, 0x50, 0x20, 0x20, 0x20, 0x00,  // Y - 89
        5, 0xF8, 0x08, 0x10, 0x20, 0x40, 0x80, 0xF8, 0x00,  // Z - 90
        2, 0xC0, 0x80, 0x80, 0x80, 0x80, 0x80, 0xC0, 0x00,  // [ - 91
        6, 0x00, 0x80, 0x40, 0x20, 0x10, 0x08, 0x04, 0x00,  //   - 92 (backslash)
        2, 0xc0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xc0, 0x00,  // ] - 93
        3, 0x40, 0xa0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ^ - 94
        5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xf0, 0x00,  // _ - 95
        2, 0xc0, 0x80, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,  // ` - 96
        5, 0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,  // a - 97
        5, 0x00, 0x80, 0x80, 0xf0, 0x88, 0x88, 0xf0, 0x00,  // b - 98
        5, 0x00, 0x00, 0x70, 0x88, 0x80, 0x88, 0x70, 0x00,  // c - 99
        5, 0x00, 0x08, 0x08, 0x78, 0x88, 0x88, 0x78, 0x00,  // d - 100
        5, 0x00, 0x00, 0x70, 0x88, 0xf8, 0x80, 0x70, 0x00,  // e - 101
        4, 0x00, 0x20, 0x50, 0x40, 0xe0, 0x40, 0x40, 0x00,  // f - 102
        5, 0x00, 0x00, 0x60, 0x90, 0x90, 0x70, 0x10, 0x60,  // g - 103
        4, 0x80, 0x80, 0x80, 0xe0, 0x90, 0x90, 0x90, 0x00,  // h - 104
        3, 0x00, 0x40, 0x00, 0xc0, 0x40, 0x40, 0xe0, 0x00,  // i - 105
        3, 0x00, 0x20, 0x00, 0x60, 0x20, 0x20, 0xa0, 0x40,  // j - 106
        4, 0x80, 0x80, 0x90, 0xa0, 0xc0, 0xa0, 0x90, 0x00,  // k - 107
        3, 0x00, 0xc0, 0x40, 0x40, 0x40, 0x40, 0xe0, 0x00,  // l - 108
        5, 0x00, 0x00, 0xd0, 0xa8, 0xa8, 0x88, 0x88, 0x00,  // m - 109
        5, 0x00, 0x00, 0xb0, 0xc8, 0x88, 0x88, 0x88, 0x00,  // n - 110
        5, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00,  // o - 111
        5, 0x00, 0x00, 0xf0, 0x88, 0x88, 0xf0, 0x80, 0x80,  // p - 112
        5, 0x00, 0x00, 0x78, 0x88, 0x88, 0x78, 0x08, 0x08,  // q - 113
        5, 0x00, 0x00, 0xb0, 0xc8, 0x80, 0x80, 0x80, 0x00,  // r - 114
        4, 0x00, 0x00, 0x70, 0x80, 0x60, 0x10, 0xE0, 0x00,  // s - 115
        4, 0x00, 0x40, 0x40, 0xe0, 0x40, 0x40, 0x50, 0x20,  // t - 116
        5, 0x00, 0x00, 0x88, 0x88, 0x88, 0x98, 0x68, 0x00,  // u - 117
        5, 0x00, 0x00, 0x88, 0x88, 0x88, 0x50, 0x20, 0x00,  // v - 118
        5, 0x00, 0x00, 0x88, 0x88, 0xa8, 0xa8, 0x50, 0x00,  // w - 119
        5, 0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00,  // x - 120
        4, 0x00, 0x00, 0x90, 0x90, 0x90, 0x70, 0x10, 0x60,  // y - 121
        5, 0x00, 0x00, 0xf8, 0x10, 0x20, 0x40, 0xf8, 0x00,  // z - 122
        3, 0x20, 0x40, 0x40, 0x80, 0x40, 0x40, 0x20, 0x00,  // { - 123
        1, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,  // | - 124
        3, 0x80, 0x40, 0x40, 0x20, 0x40, 0x40, 0x80, 0x00,  // } - 125
        5, 0x40, 0xa8, 0x10, 0x00, 0x00, 0x00, 0x00, 0x00,  // ~ - 126
        0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 127 (Delete)
        7, 0xF0, 0x40, 0x7C, 0x42, 0x42, 0x62, 0x5C, 0x00,  // ? - 128
        6, 0x20, 0xF8, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 129
        2, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xC0, 0x40,  // ? - 130
        5, 0x00, 0x20, 0xF8, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 131
        5, 0x00, 0x00, 0x00, 0x00, 0x00, 0xd8, 0x48, 0x90,  // ? - 132
        5, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0xa8,  // ? - 133
        5, 0x20, 0x20, 0xf8, 0x20, 0x20, 0x20, 0x20, 0x20,  // ? - 134
        5, 0x20, 0x20, 0xf8, 0x20, 0x20, 0xf8, 0x20, 0x20,  // ? - 135
        5, 0x30, 0x40, 0x80, 0xC0, 0x80, 0x40, 0x30, 0x00,  // ? - 136
        8, 0x00, 0xC4, 0xC8, 0x10, 0x20, 0x5B, 0x9B, 0x00,  // ? - 137
        8, 0x38, 0x48, 0x48, 0x4E, 0x49, 0x49, 0x8E, 0x00,  // ? - 138
        2, 0x00, 0x00, 0x40, 0x80, 0x40, 0x00, 0x00, 0x00,  // ? - 139
        8, 0x88, 0x88, 0x88, 0xFE, 0x89, 0x89, 0x8E, 0x00,  // ? - 140
        6, 0xA4, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x84, 0x00,  // ? - 141
        6, 0xF8, 0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x00,  // ? - 142
        5, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x20,  // ? - 143
        6, 0x40, 0xE0, 0x40, 0x58, 0x64, 0x44, 0x44, 0x08,  // ? - 144
        2, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 145
        2, 0x40, 0x80, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 146
        3, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 147
        3, 0xA0, 0xA0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 148
        2, 0x00, 0x00, 0x00, 0xC0, 0xC0, 0x00, 0x00, 0x00,  // ? - 149
        4, 0x00, 0x00, 0x00, 0x00, 0xF0, 0x00, 0x00, 0x00,  // ? - 150
        5, 0x00, 0x00, 0x00, 0x00, 0xF8, 0x00, 0x00, 0x00,  // ? - 151
        0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 152 (Non printable)
        8, 0xF1, 0x5B, 0x55, 0x51, 0x00, 0x00, 0x00, 0x00,  // ? - 153
        8, 0x38, 0x48, 0x48, 0x4E, 0x49, 0x49, 0x8E, 0x00,  // ? - 154
        2, 0x00, 0x00, 0x80, 0x40, 0x80, 0x00, 0x00, 0x00,  // ? - 155
        8, 0x88, 0x88, 0x88, 0xFE, 0x89, 0x89, 0x8E, 0x00,  // ? - 156
        6, 0xA4, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x84, 0x00,  // ? - 157
        6, 0xF8, 0x40, 0x40, 0x78, 0x44, 0x44, 0x44, 0x00,  // ? - 158
        5, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xF8, 0x20,  // ? - 159
        0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 160 (Non printable)
        5, 0x20, 0x88, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60,  // ? - 161
        5, 0x20, 0x88, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60,  // ? - 162
        6, 0x40, 0xE0, 0x40, 0x58, 0x64, 0x44, 0x44, 0x08,  // ? - 163
        5, 0x00, 0x88, 0x70, 0x50, 0x70, 0x88, 0x00, 0x00,  // ? - 164
        5, 0x08, 0xF8, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 165
        1, 0x80, 0x80, 0x80, 0x00, 0x80, 0x80, 0x80, 0x00,  // ? - 166
        4, 0x60, 0x90, 0x80, 0x60, 0x60, 0x10, 0x90, 0x60,  // ? - 167
        5, 0x50, 0xF8, 0x80, 0x80, 0xE0, 0x80, 0xF8, 0x00,  // ? - 168
        8, 0x3C, 0x42, 0x99, 0xA1, 0xA1, 0x99, 0x42, 0x3C,  // ? - 169
        5, 0x70, 0x88, 0x80, 0xE0, 0x80, 0x88, 0x70, 0x00,  // ? - 170
        6, 0x00, 0x24, 0x48, 0x90, 0x48, 0x24, 0x00, 0x00,  // ? - 171
        5, 0xF8, 0x08, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 172
        0, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  //   - 173 (Non printable)
        8, 0x3C, 0x42, 0xBD, 0xA5, 0xB9, 0xA5, 0x42, 0x3C,  // ? - 174
        3, 0xA0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,  // ? - 175
        3, 0x40, 0xA0, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00,  // ? - 176
        5, 0x20, 0x20, 0xF8, 0x20, 0x20, 0x00, 0xF8, 0x00,  // ? - 177
        3, 0xE0, 0x40, 0x40, 0x40, 0x40, 0x40, 0xE0, 0x00,  // ? - 178
        3, 0x00, 0x40, 0x00, 0xc0, 0x40, 0x40, 0xe0, 0x00,  // ? - 179
        4, 0x00, 0x10, 0xF0, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 180
        5, 0x00, 0x88, 0x88, 0x88, 0x88, 0x98, 0xE8, 0x80,  // ? - 181
        6, 0xF8, 0x50, 0x50, 0x50, 0x50, 0x54, 0x48, 0x00,  // ? - 182
        1, 0x00, 0x00, 0x00, 0x80, 0x00, 0x00, 0x00, 0x00,  // ? - 183
        5, 0x00, 0x50, 0x70, 0x88, 0xF8, 0x80, 0x70, 0x00,  // ? - 184
        8, 0x88, 0x88, 0xCB, 0xAB, 0x98, 0x88, 0x8B, 0x00,  // ? - 185
        5, 0x00, 0x00, 0x70, 0x88, 0xE0, 0x88, 0x70, 0x00,  // ? - 186
        6, 0x00, 0x90, 0x48, 0x24, 0x48, 0x90, 0x00, 0x00,  // ? - 187
        3, 0x00, 0x20, 0x00, 0x60, 0x20, 0x20, 0xa0, 0x40,  // ? - 188
        5, 0x78, 0x80, 0x80, 0x70, 0x08, 0x08, 0xF0, 0x00,  // ? - 189
        4, 0x00, 0x00, 0x70, 0x80, 0x60, 0x10, 0xE0, 0x00,  // ? - 190
        3, 0x00, 0xA0, 0x00, 0xC0, 0x40, 0x40, 0xE0, 0x00,  // ? - 191
        5, 0x38, 0x48, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,  // ? - 192
        5, 0xF8, 0x80, 0x80, 0xF0, 0x88, 0x88, 0xF0, 0x00,  // ? - 193
        5, 0xF0, 0x88, 0x88, 0xF0, 0x88, 0x88, 0xF0, 0x00,  // ? - 194
        4, 0xF0, 0x80, 0x80, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 195
        6, 0x38, 0x48, 0x48, 0x48, 0x48, 0x48, 0xFC, 0x84,  // ? - 196
        5, 0xF8, 0x80, 0x80, 0xE0, 0x80, 0x80, 0xF8, 0x00,  // ? - 197
        7, 0x92, 0x92, 0x54, 0x38, 0x54, 0x92, 0x92, 0x00,  // ? - 198
        5, 0x70, 0x88, 0x08, 0x30, 0x08, 0x88, 0x70, 0x00,  // ? - 199
        5, 0x88, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x88, 0x00,  // ? - 200
        5, 0x20, 0x88, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x00,  // ? - 201
        5, 0x88, 0x90, 0xA0, 0xC0, 0xA0, 0x90, 0x88, 0x00,  // ? - 202
        5, 0x38, 0x48, 0x88, 0x88, 0x88, 0x88, 0x88, 0x00,  // ? - 203
        5, 0x88, 0xD8, 0xA8, 0x88, 0x88, 0x88, 0x88, 0x00,  // ? - 204
        5, 0x88, 0x88, 0x88, 0xf8, 0x88, 0x88, 0x88, 0x00,  // ? - 205
        5, 0x70, 0x88, 0x88, 0x88, 0x88, 0x88, 0x70, 0x00,  // ? - 206
        5, 0xF8, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0x00,  // ? - 207
        5, 0xf0, 0x88, 0x88, 0x88, 0xf0, 0x80, 0x80, 0x00,  // ? - 208
        5, 0x70, 0x88, 0x80, 0x80, 0x80, 0x88, 0x70, 0x00,  // ? - 209
        5, 0xf8, 0x20, 0x20, 0x20, 0x20, 0x20, 0x20, 0x00,  // ? - 210
        5, 0x88, 0x88, 0x88, 0x78, 0x08, 0x10, 0x60, 0x00,  // ? - 211
        7, 0x10, 0x7C, 0x92, 0x92, 0x92, 0x7C, 0x10, 0x10,  // ? - 212
        5, 0x88, 0x88, 0x50, 0x20, 0x50, 0x88, 0x88, 0x00,  // ? - 213
        6, 0x88, 0x88, 0x88, 0x88, 0x88, 0x88, 0xFC, 0x04,  // ? - 214
        5, 0x88, 0x88, 0x88, 0xF8, 0x08, 0x08, 0x08, 0x00,  // ? - 215
        7, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0xFE, 0x00,  // ? - 216
        8, 0x92, 0x92, 0x92, 0x92, 0x92, 0x92, 0xFF, 0x01,  // ? - 217
        6, 0xC0, 0x40, 0x40, 0x78, 0x44, 0x44, 0x78, 0x00,  // ? - 218
        7, 0x82, 0x82, 0x82, 0xF2, 0x8A, 0x8A, 0xF2, 0x00,  // ? - 219
        5, 0x80, 0x80, 0x80, 0xF0, 0x88, 0x88, 0xF0, 0x00,  // ? - 220
        5, 0x70, 0x88, 0x08, 0x38, 0x08, 0x88, 0x70, 0x00,  // ? - 221
        7, 0x9C, 0xA2, 0xA2, 0xE2, 0xA2, 0xA2, 0x9C, 0x00,  // ? - 222
        5, 0x78, 0x88, 0x88, 0x88, 0x78, 0x48, 0x88, 0x00,  // ? - 223
        5, 0x00, 0x00, 0x70, 0x08, 0x78, 0x88, 0x78, 0x00,  // ? - 224
        5, 0x00, 0x70, 0x80, 0xF0, 0x88, 0x88, 0x70, 0x00,  // ? - 225
        5, 0x00, 0x00, 0xF0, 0x88, 0xF0, 0x88, 0xF0, 0x00,  // ? - 226
        4, 0x00, 0x00, 0xF0, 0x80, 0x80, 0x80, 0x80, 0x00,  // ? - 227
        5, 0x00, 0x00, 0x30, 0x50, 0x50, 0x50, 0xF8, 0x88,  // ? - 228
        5, 0x00, 0x00, 0x70, 0x88, 0xf8, 0x80, 0x70, 0x00,  // ? - 229
        5, 0x00, 0x00, 0xA8, 0xA8, 0x70, 0xA8, 0xA8, 0x00,  // ? - 230
        4, 0x00, 0x00, 0xE0, 0x10, 0x60, 0x10, 0xE0, 0x00,  // ? - 231
        5, 0x00, 0x00, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x00,  // ? - 232
        5, 0x00, 0x20, 0x88, 0x98, 0xA8, 0xC8, 0x88, 0x00,  // ? - 233
        5, 0x00, 0x00, 0x88, 0x90, 0xE0, 0x90, 0x88, 0x00,  // ? - 234
        5, 0x00, 0x00, 0x38, 0x48, 0x48, 0x48, 0x88, 0x00,  // ? - 235
        5, 0x00, 0x00, 0x88, 0xD8, 0xA8, 0x88, 0x88, 0x00,  // ? - 236
        5, 0x00, 0x00, 0x88, 0x88, 0xF8, 0x88, 0x88, 0x00,  // ? - 237
        5, 0x00, 0x00, 0x70, 0x88, 0x88, 0x88, 0x70, 0x00,  // ? - 238
        5, 0x00, 0x00, 0xF8, 0x88, 0x88, 0x88, 0x88, 0x00,  // ? - 239
        5, 0x00, 0x00, 0xF0, 0x88, 0x88, 0xF0, 0x80, 0x80,  // ? - 240
        4, 0x00, 0x00, 0x70, 0x80, 0x80, 0x80, 0x70, 0x00,  // ? - 241
        5, 0x00, 0x00, 0xF8, 0x20, 0x20, 0x20, 0x20, 0x00,  // ? - 242
        5, 0x00, 0x00, 0x88, 0x88, 0x88, 0x78, 0x08, 0x70,  // ? - 243
        5, 0x00, 0x00, 0x70, 0xA8, 0xA8, 0x70, 0x20, 0x20,  // ? - 244
        5, 0x00, 0x00, 0x88, 0x50, 0x20, 0x50, 0x88, 0x00,  // ? - 245
        5, 0x00, 0x00, 0x90, 0x90, 0x90, 0x90, 0xF8, 0x08,  // ? - 246
        5, 0x00, 0x00, 0x88, 0x88, 0x78, 0x08, 0x08, 0x00,  // ? - 247
        5, 0x00, 0x00, 0x88, 0x88, 0xA8, 0xA8, 0xF8, 0x00,  // ? - 248
        6, 0x00, 0x00, 0x88, 0x88, 0xA8, 0xA8, 0xFC, 0x04,  // ? - 249
        5, 0x00, 0x00, 0xC0, 0x40, 0x70, 0x48, 0x70, 0x00,  // ? - 250
        6, 0x00, 0x00, 0x84, 0x84, 0xE4, 0x94, 0xE4, 0x00,  // ? - 251
        4, 0x00, 0x00, 0x80, 0x80, 0xE0, 0x90, 0xE0, 0x00,  // ? - 252
        5, 0x00, 0x00, 0x70, 0x88, 0x38, 0x88, 0x70, 0x00,  // ? - 253
        6, 0x00, 0x00, 0x98, 0xA4, 0xE4, 0xA4, 0x98, 0x00,  // ? - 254
        5, 0x00, 0x00, 0x78, 0x88, 0x78, 0x88, 0x88, 0x00,  // ? - 255
    };

dy_gfx_font_t dy_gfx_font_8x8v1 = {
    .ascii_offset = 32,
    .length = 224,  // last_char - ascii_offset + 1
    .width = DY_GFX_FONT_SYMBOL_WIDTH_8,
    .height = 8,
    .content.c8 = data,
};
