#include "c5_bitmap_text_renderer.h"

#include <ctype.h>
#include <stddef.h>
#include <string.h>

#define GLYPH_W 5
#define GLYPH_H 7

static const uint8_t GLYPH_SPACE[GLYPH_H] = {0,0,0,0,0,0,0};
static const uint8_t GLYPH_BOX[GLYPH_H] = {31,17,17,17,17,17,31};

static const uint8_t *glyph_for(char raw) {
    static const uint8_t A[7]={14,17,17,31,17,17,17}; static const uint8_t B[7]={30,17,17,30,17,17,30};
    static const uint8_t C[7]={14,17,16,16,16,17,14}; static const uint8_t D[7]={30,17,17,17,17,17,30};
    static const uint8_t E[7]={31,16,16,30,16,16,31}; static const uint8_t F[7]={31,16,16,30,16,16,16};
    static const uint8_t G[7]={14,17,16,23,17,17,15}; static const uint8_t H[7]={17,17,17,31,17,17,17};
    static const uint8_t I[7]={14,4,4,4,4,4,14}; static const uint8_t J[7]={7,2,2,2,18,18,12};
    static const uint8_t K[7]={17,18,20,24,20,18,17}; static const uint8_t L[7]={16,16,16,16,16,16,31};
    static const uint8_t M[7]={17,27,21,21,17,17,17}; static const uint8_t N[7]={17,25,21,19,17,17,17};
    static const uint8_t O[7]={14,17,17,17,17,17,14}; static const uint8_t P[7]={30,17,17,30,16,16,16};
    static const uint8_t Q[7]={14,17,17,17,21,18,13}; static const uint8_t R[7]={30,17,17,30,20,18,17};
    static const uint8_t S[7]={15,16,16,14,1,1,30}; static const uint8_t T[7]={31,4,4,4,4,4,4};
    static const uint8_t U[7]={17,17,17,17,17,17,14}; static const uint8_t V[7]={17,17,17,17,17,10,4};
    static const uint8_t W[7]={17,17,17,21,21,21,10}; static const uint8_t X[7]={17,17,10,4,10,17,17};
    static const uint8_t Y[7]={17,17,10,4,4,4,4}; static const uint8_t Z[7]={31,1,2,4,8,16,31};

    static const uint8_t N0[7]={14,17,19,21,25,17,14}; static const uint8_t N1[7]={4,12,4,4,4,4,14};
    static const uint8_t N2[7]={14,17,1,2,4,8,31}; static const uint8_t N3[7]={30,1,1,14,1,1,30};
    static const uint8_t N4[7]={2,6,10,18,31,2,2}; static const uint8_t N5[7]={31,16,16,30,1,1,30};
    static const uint8_t N6[7]={14,16,16,30,17,17,14}; static const uint8_t N7[7]={31,1,2,4,8,8,8};
    static const uint8_t N8[7]={14,17,17,14,17,17,14}; static const uint8_t N9[7]={14,17,17,15,1,1,14};

    static const uint8_t COLON[7]={0,4,4,0,4,4,0}; static const uint8_t DOT[7]={0,0,0,0,0,12,12};
    static const uint8_t SLASH[7]={1,1,2,4,8,16,16}; static const uint8_t DASH[7]={0,0,0,31,0,0,0};
    static const uint8_t UNDER[7]={0,0,0,0,0,0,31}; static const uint8_t PERCENT[7]={24,25,2,4,8,19,3};
    static const uint8_t PLUS[7]={0,4,4,31,4,4,0}; static const uint8_t PIPE[7]={4,4,4,4,4,4,4};

    char c = (char)toupper((unsigned char)raw);
    if (c == ' ') return GLYPH_SPACE;
    if (c >= 'A' && c <= 'Z') {
        static const uint8_t *letters[26] = {A,B,C,D,E,F,G,H,I,J,K,L,M,N,O,P,Q,R,S,T,U,V,W,X,Y,Z};
        return letters[c - 'A'];
    }
    if (c >= '0' && c <= '9') {
        static const uint8_t *digits[10] = {N0,N1,N2,N3,N4,N5,N6,N7,N8,N9};
        return digits[c - '0'];
    }
    switch (c) {
        case ':': return COLON;
        case '.': return DOT;
        case '/': return SLASH;
        case '-': return DASH;
        case '_': return UNDER;
        case '%': return PERCENT;
        case '+': return PLUS;
        case '|': return PIPE;
        default: return GLYPH_BOX;
    }
}

static int normalized_scale(const c5_bitmap_text_config_t *config) {
    return (config != 0 && config->scale > 0) ? config->scale : 1;
}

static int char_spacing(const c5_bitmap_text_config_t *config) {
    return (config != 0 && config->char_spacing >= 0) ? config->char_spacing : 1;
}

static int line_spacing(const c5_bitmap_text_config_t *config) {
    return (config != 0 && config->line_spacing >= 0) ? config->line_spacing : 2;
}

static void draw_pixel_block(const c5_bitmap_text_config_t *config, int x, int y) {
    int scale = normalized_scale(config);
    config->fill_rect(x, y, scale, scale, config->fg_color, config->user_ctx);
}

int c5_bitmap_draw_text(
    const c5_bitmap_text_config_t *config,
    int x,
    int y,
    const char *text
) {
    if (config == 0 || config->fill_rect == 0 || text == 0) {
        return -1;
    }

    int scale = normalized_scale(config);
    int cursor_x = x;

    for (size_t i = 0; text[i] != '\0'; ++i) {
        const uint8_t *glyph = glyph_for(text[i]);
        for (int row = 0; row < GLYPH_H; ++row) {
            for (int col = 0; col < GLYPH_W; ++col) {
                if ((glyph[row] >> (GLYPH_W - 1 - col)) & 1) {
                    draw_pixel_block(config, cursor_x + col * scale, y + row * scale);
                }
            }
        }
        cursor_x += (GLYPH_W * scale) + char_spacing(config);
    }

    return 0;
}

int c5_bitmap_draw_frame(
    const c5_bitmap_text_config_t *config,
    int x,
    int y,
    const c5_screen_frame_t *frame
) {
    if (config == 0 || frame == 0) {
        return -1;
    }

    int scale = normalized_scale(config);
    int row_step = (GLYPH_H * scale) + line_spacing(config);

    for (int i = 0; i < C5_SCREEN_LINE_COUNT; ++i) {
        if (c5_bitmap_draw_text(config, x, y + i * row_step, frame->lines[i]) != 0) {
            return -1;
        }
    }
    return 0;
}
