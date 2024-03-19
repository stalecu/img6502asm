#ifndef IMG6502ASM_H
#define IMG6502ASM_H

#include <stdint.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

typedef uint32_t color_t;

typedef struct rgb_t {
  uint8_t r;
  uint8_t g;
  uint8_t b;
} rgb_t;

#define RGB(r, g, b) (rgb_t){(r), (g), (b)}

static const rgb_t colors[] = {
  RGB(0x00, 0x00, 0x00), // $0: black
  RGB(0xFF, 0xFF, 0xFF), // $1: white
  RGB(0x88, 0x00, 0x00), // $2: red
  RGB(0xAA, 0xFF, 0xEE), // $3: cyan
  RGB(0xCC, 0x44, 0xCC), // $4: purple
  RGB(0x00, 0xCC, 0x55), // $5: green
  RGB(0x00, 0x00, 0xAA), // $6: blue
  RGB(0xEE, 0xEE, 0x77), // $7: yellow
  RGB(0xDD, 0x88, 0x55), // $8: orange
  RGB(0x66, 0x44, 0x00), // $9: brown
  RGB(0xFF, 0x77, 0x77), // $a: light red
  RGB(0x33, 0x33, 0x33), // $b: dark grey
  RGB(0x77, 0x77, 0x77), // $c: grey
  RGB(0xAA, 0xFF, 0x66), // $d: light green
  RGB(0x00, 0x88, 0xFF), // $e: light blue
  RGB(0xBB, 0xBB, 0xBB), // $f: light grey
};

double weighted_distance(const rgb_t a, const rgb_t b);
double euclidian_distance(const rgb_t a, const rgb_t b);

typedef struct xyz_t {
  double r;
  double g;
  double b;
} xyz_t;

xyz_t rgb_to_xyz(const rgb_t col);

typedef struct lab_t {
  double L;
  double a;
  double b;
} lab_t;

lab_t xyz_to_lab(const xyz_t xyz);
double ciede2000(const lab_t lab1, const lab_t lab2);
double cielab_distance(const rgb_t a, const rgb_t b);

typedef enum ALGORITHM {
  EUCLIDIAN,
  WEIGHTED,
  CIELAB
} ALGORITHM;

double measure_similarity(const rgb_t a, const rgb_t b, const ALGORITHM algo);
int closest_color_index(const rgb_t color, const ALGORITHM algo);


#endif
