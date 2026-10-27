#define SYM_DEGREE 0xB0

typedef struct {
    uint8_t byte;
    uint8_t offset;
} segment_map_t;

// display memory offset for LED segments
// see: https://cdn-learn.adafruit.com/assets/assets/000/032/143/large1024/feather_led_matrix_segments.png?1462384756
//
//         A
//      -------
//   F |H\J|/K| B
//   G1 --- --- G2 
//   E |L/M|\N| C
//      -------
//         D

// segments sequence: l m n  k j h g2 g1 f e d c b a
segment_map_t digit_map[SEGMENTS_PER_DIGIT] = {
    {0,0},
    {2,0},
    {4,0},
    {6,0},
    {8,0},
    {10,0},
    {12,0},
    {0,4},
    {2,4},
    {4,4},
    {6,4},
    {8,4},
    {10,4},
    {12,4},
};

static const uint16_t alpha_num_font[] = {
// Ziffern
    ['0'] = 0x243F, // 0010 0100 0011 1111 (A,B,C,D,E,F + J, M)
    ['1'] = 0x0406, // 0000 0100 0000 0110 (B,C + J)
    ['2'] = 0x00DB, // 0000 0000 1101 1011 (A,B,D,E + G1,G2)
    ['3'] = 0x00CF, // 0000 0000 1100 1111 (A,B,C,D + G1,G2)
    ['4'] = 0x00E6, // 0000 0000 1110 0110 (B,C,F + G1,G2)
    ['5'] = 0x00ED, // 0000 0000 1110 1101 (A,C,D,F + G1,G2)
    ['6'] = 0x00FD, // 0000 0000 1111 1101 (A,C,D,E,F + G1,G2)
    ['7'] = 0x0007, // 0000 0000 0000 0111 (A,B,C)
    ['8'] = 0x00FF, // 0000 0000 1111 1111 (Alle Außen + G1,G2)
    ['9'] = 0x00EF, // 0000 0000 1110 1111 (A,B,C,D,F + G1,G2)

    // Buchstaben A-Z
    ['A'] = 0x00F7, // 0000 0000 1111 0111 (A,B,C,E,F + G1,G2)
    ['B'] = 0x128F, // 0001 0010 1000 1111 (A,B,C,D + G2 + I,L)
    ['C'] = 0x0039, // 0000 0000 0011 1001 (A,D,E,F)
    ['D'] = 0x120F, // 0001 0010 0000 1111 (A,B,C,D + I,L)
    ['E'] = 0x00F9, // 0000 0000 1111 1001 (A,D,E,F + G1,G2)
    ['F'] = 0x00F1, // 0000 0000 1111 0001 (A,E,F + G1,G2)
    ['G'] = 0x00BD, // 0000 0000 1011 1101 (A,C,D,E,F + G2)
    ['H'] = 0x00F6, // 0000 0000 1111 0110 (B,C,E,F + G1,G2)
    ['I'] = 0x1209, // 0001 0010 0000 1001 (A,D + I,L)
    ['J'] = 0x001E, // 0000 0000 0001 1110 (B,C,D,E)
    ['K'] = 0x0C70, // 0000 1100 0111 0000 (E,F + G1 + J,K - Diagonale Rechts)
    ['L'] = 0x0038, // 0000 0000 0011 1000 (D,E,F)
    ['M'] = 0x0536, // 0000 0101 0011 0110 (B,C,E,F + H,J - V-Form Oben)
    ['N'] = 0x0936, // 0000 1001 0011 0110 (B,C,E,F + H,K - Diagonale von Links-Oben nach Rechts-Unten)
    ['O'] = 0x003F, // 0000 0000 0011 1111 (A,B,C,D,E,F)
    ['P'] = 0x00F3, // 0000 0000 1111 0011 (A,B,E,F + G1,G2)
    ['Q'] = 0x083F, // 0000 1000 0011 1111 (A,B,C,D,E,F + K - Rechts Unten)
    ['R'] = 0x08F3, // 0000 1000 1111 0011 (A,B,E,F + G1,G2 + K - Rechts Unten)
    ['S'] = 0x00ED, // 0000 0000 1110 1101 (A,C,D,F + G1,G2)
    ['T'] = 0x1201, // 0001 0010 0000 0001 (A + I,L)
    ['U'] = 0x003E, // 0000 0000 0011 1110 (B,C,D,E,F)
    ['V'] = 0x2430, // 0010 0100 0011 0000 (E,F + J,M - Diagonale Oben Rechts/Unten Links)
    ['W'] = 0x2836, // 0010 1000 0011 0110 (B,C,E,F + M,K - V-Form Unten)
    ['X'] = 0x2D00, // 0010 1101 0000 0000 (H,J,K,M - Alle Diagonalen)
    ['Y'] = 0x1500, // 0001 0101 0000 0000 (H,J + L - Y-Form mit Vertikal Unten)
    ['Z'] = 0x2409, // 0010 0100 0000 1001 (A,D + J,M - Diagonale Links/Rechts Oben)
    
    // Symbole
    [' '] = 0x0000,
    ['-'] = 0x00C0,
    [SYM_DEGREE] = 0x00E3,

};




