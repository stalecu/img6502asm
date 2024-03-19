#include "img6502asm.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <png.h>

#define PNG_BYTES_TO_CHECK 8

int process_image(const char *input_filename, const char *output_filename,
                  const char *algorithm, const char *asm_filename);

void print_help(const char *programName);

int main(int argc, char **argv) {
  const char *input_filename = NULL;
  const char *output_filename = "output.png";
  const char *algorithm = "cielab";
  const char *asm_filename = NULL;

  // Parse options
  for (int i = 1; i < argc; i++) {
    if (strcmp(argv[i], "-i") == 0 || strcmp(argv[i], "--input") == 0) {
      if (i + 1 < argc) {
        input_filename = argv[++i];
      } else {
        fprintf(stderr, "Error: Missing argument for option -i/--input\n");
        return 1;
      }
    } else if (strcmp(argv[i], "-o") == 0 || strcmp(argv[i], "--output") == 0) {
      if (i + 1 < argc) {
        output_filename = argv[++i];
      } else {
        fprintf(stderr, "Error: Missing argument for option -o/--output\n");
        return 1;
      }
    } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--asm") == 0) {
      if (i + 1 < argc) {
        if (argv[i + 1][0] == '-') {
          asm_filename = NULL;
          continue;
        }
        asm_filename = argv[++i];
      } else {
        fprintf(stderr, "Error: Missing argument for option -s/--asm\n");
        return 1;
      }
    } else if (strcmp(argv[i], "-a") == 0 ||
               strcmp(argv[i], "--algorithm") == 0) {
      if (i + 1 < argc) {
        algorithm = argv[++i];
        // Check if algorithm is valid
        if (strcmp(algorithm, "cielab") != 0 &&
            strcmp(algorithm, "weighted") != 0 &&
            strcmp(algorithm, "euclidian") != 0) {
          fprintf(stderr,
                  "Error: Invalid value '%s' for option -a/--algorithm\n",
                  algorithm);
          return 1;
        }
      } else {
        fprintf(stderr, "Error: Missing argument for option -a/--algorithm\n");
        return 1;
      }
    } else if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
      print_help(argv[0]);
      return 0;
    } else {
      fprintf(stderr, "Error: Unknown option '%s'\n", argv[i]);
      return 1;
    }
  }

  // Check if input file is provided
  if (input_filename == NULL) {
    fprintf(stderr, "Error: Input file not provided.\n");
    return 1;
  }

  // Process PNG
  return process_image(input_filename, output_filename, algorithm,
                       asm_filename);
}

void print_help(const char *programName) {
  printf("Usage: %s [OPTIONS]\n", programName);
  printf("Converts the given PNG image to 6502 assembly intended for use in "
         "Easy6502.\n");
  printf("\n");
  printf("Options:\n");
  printf("  -i, --input         Path of the input PNG file.\n");
  printf("  -o, --output        Path of the output PNG file (default: "
         "output.png).\n");
  printf("  -s, --asm           Path of the output assembly file (default: "
         "stdout).\n");
  printf("  -a, --algorithm     The algorithm used for choosing the colors "
         "(options: cielab, weighted, euclidian). Default: cielab.\n");
  printf("  -h, --help          Display this help message and exit.\n");
}

int process_image(const char *filename, const char *output_filename,
                  const char *algorithm, const char *asm_filename) {
  FILE *fp = fopen(filename, "rb");
  if (!fp) {
    printf("Error: File %s could not be opened for reading\n", filename);
    return 1;
  }

  png_byte header[8];
  fread(header, 1, 8, fp);
  int is_png = !png_sig_cmp(header, 0, PNG_BYTES_TO_CHECK);
  if (!is_png) {
    fclose(fp);
    printf("Error: File %s is not a PNG image\n", filename);
    return 1;
  }

  png_structp png_ptr =
      png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_ptr) {
    fclose(fp);
    printf("Error: png_create_read_struct failed\n");
    return 1;
  }

  png_infop info_ptr = png_create_info_struct(png_ptr);
  if (!info_ptr) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, NULL, NULL);
    printf("Error: png_create_info_struct failed\n");
    return 1;
  }

  png_bytep *row_pointers = NULL;
  if (setjmp(png_jmpbuf(png_ptr))) {
    fclose(fp);
    png_destroy_read_struct(&png_ptr, &info_ptr, NULL);
    if (row_pointers)
      free(row_pointers);
    printf("Error: Error during reading PNG file\n");
    return 1;
  }

  png_init_io(png_ptr, fp);
  png_set_sig_bytes(png_ptr, PNG_BYTES_TO_CHECK);
  png_read_info(png_ptr, info_ptr);

  int width = png_get_image_width(png_ptr, info_ptr);
  int height = png_get_image_height(png_ptr, info_ptr);
  png_byte color_type = png_get_color_type(png_ptr, info_ptr);
  png_byte bit_depth = png_get_bit_depth(png_ptr, info_ptr);

  if (width > 32 || height > 32) {
    fprintf(stderr, "Image %s is greater than 32x32.", filename);
    return 1;
  }

  // Force the image to be read as RGB
  if (color_type == PNG_COLOR_TYPE_PALETTE)
    png_set_palette_to_rgb(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
    png_set_expand_gray_1_2_4_to_8(png_ptr);
  if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
    png_set_tRNS_to_alpha(png_ptr);
  if (bit_depth == 16)
    png_set_strip_16(png_ptr);
  if (color_type == PNG_COLOR_TYPE_GRAY ||
      color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
    png_set_gray_to_rgb(png_ptr);
  if (color_type == PNG_COLOR_TYPE_RGB_ALPHA)
    png_set_swap_alpha(png_ptr);

  // Allocate memory for image data
  row_pointers = (png_bytep *)malloc(sizeof(png_bytep) * height);
  for (int y = 0; y < height; y++)
    row_pointers[y] = (png_byte *)malloc(png_get_rowbytes(png_ptr, info_ptr));

  // Read image data
  png_read_image(png_ptr, row_pointers);
  fclose(fp);

  // Writing new image
  FILE *output_fp = fopen(output_filename, "wb");
  if (!output_fp) {
    printf("Error: File %s could not be opened for writing\n", output_filename);
    return 1;
  }

  png_structp png_write_ptr =
      png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
  if (!png_write_ptr) {
    fclose(output_fp);
    printf("Error: png_create_write_struct failed\n");
    return 1;
  }

  png_infop info_write_ptr = png_create_info_struct(png_write_ptr);
  if (!info_write_ptr) {
    fclose(output_fp);
    png_destroy_write_struct(&png_write_ptr, NULL);
    printf("Error: png_create_info_struct failed\n");
    return 1;
  }

  if (setjmp(png_jmpbuf(png_write_ptr))) {
    fclose(output_fp);
    png_destroy_write_struct(&png_write_ptr, &info_write_ptr);
    printf("Error: Error during writing PNG file\n");
    return 1;
  }

  png_init_io(png_write_ptr, output_fp);
  png_set_IHDR(png_write_ptr, info_write_ptr, width, height, 8,
               PNG_COLOR_TYPE_RGB, PNG_INTERLACE_NONE,
               PNG_COMPRESSION_TYPE_BASE, PNG_FILTER_TYPE_BASE);
  png_write_info(png_write_ptr, info_write_ptr);

  ALGORITHM algo;
  if (strcmp(algorithm, "cielab") || algorithm == NULL) {
    algo = CIELAB;
  } else if (strcmp(algorithm, "euclidian")) {
    algo = EUCLIDIAN;
  } else if (strcmp(algorithm, "weighted")) {
    algo = WEIGHTED;
  }

  FILE *asm;
  if (asm_filename != NULL) {
    asm = fopen(asm_filename, "w");
    if (!asm) {
      fprintf(stderr, "Error: Cannot open %s for writing.\n", asm_filename);
      return 1;
    }
  } else {
    asm = stdout;
  }

  int i = 0;
  for (int y = 0; y < height; y++) {
    png_bytep row = row_pointers[y];
    for (int x = 0; x < width; x++) {
      png_bytep px = &(row[x * 3]);

      rgb_t orig_color = {px[0], px[1], px[2]};

      int closest_idx = closest_color_index(orig_color, algo);
      rgb_t color = colors[closest_idx];
      px[0] = color.r;
      px[1] = color.g;
      px[2] = color.b;

      fprintf(asm, "LDA #$%02x\n", closest_idx);
      fprintf(asm, "STA $%04x\n", 512 + i++);
    }
  }

  // Write the image
  png_write_image(png_write_ptr, row_pointers);
  png_write_end(png_write_ptr, NULL);

  // Cleanup
  fclose(output_fp);
  for (int y = 0; y < height; y++)
    free(row_pointers[y]);
  free(row_pointers);
  png_destroy_write_struct(&png_write_ptr, &info_write_ptr);

  return 0;
}
