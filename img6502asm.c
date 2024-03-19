#include "img6502asm.h"
#include <math.h>
#include <stdlib.h>

// Calculate the weighted distance between two RGB colors
double weighted_distance(const rgb_t a, const rgb_t b) {
  const double WEIGHT_R = 0.2989;
  const double WEIGHT_G = 0.5870;
  const double WEIGHT_B = 0.1140;
    
  return WEIGHT_R * abs(a.r - b.r) +
	 WEIGHT_G * abs(a.g - b.g) +
	 WEIGHT_B * abs(a.b - b.b);
}

// Calculate the Euclidean distance between two RGB colors
double euclidian_distance(const rgb_t a, const rgb_t b) {
  return sqrt(pow(a.r - b.r, 2) + pow(a.g - b.g, 2) + pow(a.b - b.b, 2));
}

// Convert an RGB color to CIE 1931 XYZ color space
xyz_t rgb_to_xyz(const rgb_t col) {
  // Normalize the colors to be in [0, 1]
  xyz_t xyz = {
    .r = (double)(col.r) / 255.0,
    .g = (double)(col.g) / 255.0,
    .b = (double)(col.b) / 255.0
  };

    // Convert linear RGB to nonlinear RGB
  if (xyz.r > 0.04045) {
    xyz.r = pow((xyz.r + 0.055) / 1.055, 2.4);
  } else {
    xyz.r /= 12.92;
  }

  // Repeat for green and blue components
  if (xyz.g > 0.04045) {
    xyz.g = pow((xyz.g + 0.055) / 1.055, 2.4);
  } else {
    xyz.g /= 12.92;
  }

  if (xyz.b > 0.04045) {
    xyz.b = pow((xyz.b + 0.055) / 1.055, 2.4);
  } else {
    xyz.b /= 12.92;
  }

  xyz.r *= 100.0;
  xyz.g *= 100.0;
  xyz.b *= 100.0;

  const double w[3][3] = {
    { 0.4124564, 0.3575761, 0.1804375 },
    { 0.2126729, 0.7151522, 0.0721750 },
    { 0.0193339, 0.1191920, 0.9503041 }
  };
  
  return (xyz_t){
    .r = xyz.r * w[0][0] + xyz.g * w[0][1] + xyz.b * w[0][2],
    .g = xyz.r * w[1][0] + xyz.g * w[1][1] + xyz.b * w[1][2],
    .b = xyz.r * w[2][0] + xyz.g * w[2][1] + xyz.b * w[2][2],
  };
}

// Convert CIE 1931 XYZ color space to CIE LAB color space
lab_t xyz_to_lab(const xyz_t xyz) {
  // Define reference values (D65 white)
  const double refs[3] = {95.0489, 100.0, 108.883};
  lab_t lab = {
    .L = xyz.r / refs[0],
    .a = xyz.g / refs[1],
    .b = xyz.b / refs[2]
  };

  const double d3 = 0.00885645171;
  const double t0 = 7.78703702;

  if (lab.L > d3) {
    lab.L = pow(lab.L, 1/3.0);
  } else {
    lab.L = t0 * lab.L + 4.0 / 29;
  }

  if (lab.a > d3) {
    lab.a = pow(lab.a, 1/3.0);
  } else {
    lab.a = t0 * lab.a + 4.0 / 29;
  }

  if (lab.b > d3) {
    lab.b = pow(lab.b, 1/3.0);
  } else {
    lab.b = t0 * lab.b + 4.0 / 29;
  }

  return (lab_t){
    .L = (116 * lab.a) - 16.0,
    .a = 500.0 * (lab.L - lab.a),
    .b = 200 * (lab.a - lab.b)
  };
}

double ciede2000(const lab_t lab1, const lab_t lab2) {
  const double C_25_7 = 6103515625.0;
  const double PI  = 3.14159265358979323846;
  const double TAU = 6.28318530717958647692;
  const double PI_OVER_6 =  0.52359877559829887307;
  const double PI_OVER_30 = 0.104719755119659774615333;
  const double DEG_RAD = 0.01745329251994329577;
  const double RAD_DEG = 57.2957795130823208768;

  // Extract LAB components
  const double L1 = lab1.L, a1 = lab1.a, b1 = lab1.b;
  const double L2 = lab2.L, a2 = lab2.a, b2 = lab2.b;

  // Calculate CIELAB differences
  double C1 = sqrt(a1 * a1 + b1 * b1);
  double C2 = sqrt(a2 * a2 + b2 * b2);
  double C_ave = (C1 + C2) / 2;

  // Calculate G factor
  double G = 0.5 * (1 - sqrt(pow(C_ave, 7) / (pow(C_ave, 7) + C_25_7)));
  
  // Transform a and b components
  double a1_ = (1 + G) * a1;
  double a2_ = (1 + G) * a2;

  // Calculate C1_ and C2_
  double C1_ = sqrt(a1_ * a1_ + b1 * b1);
  double C2_ = sqrt(a2_ * a2_ + b2 * b2);

  // Calculate h1_ and h2_
  double h1_ = atan2(b1, a1_) + (b1 >= 0 ? 0 : TAU);
  double h2_ = atan2(b2, a2_) + (b2 >= 0 ? 0 : TAU);

  // Calculate ΔL, ΔC, Δh, ΔH
  double dL_ = L2 - L1;
  double dC_ = C2_ - C1_;
  double dh_ = fabs(h2_ - h1_);
  if (C1_ * C2_ != 0) {
    if (dh_ > PI) dh_ -= TAU;
    else if (dh_ < -PI) dh_ += TAU;
  }
  double dH_ = 2 * sqrt(C1_ * C2_) * sin(dh_ / 2);

  // Calculate L_ave, h_ave
  double L_ave = (L1 + L2) / 2;
  double h_ave = (C1_ * C2_ == 0) ? h1_ + h2_ : (fabs(h1_ - h2_) <= PI) ? (h1_ + h2_) / 2 : (h1_ + h2_ + 2 * PI) / 2;

  // Calculate weighting factors
  double T = 1 - 0.17 * cos(h_ave - PI_OVER_6) + 0.24 * cos(2 * h_ave) + 0.32 * cos(3 * h_ave + PI_OVER_30) - 0.2 * cos(4 * h_ave - 63 * DEG_RAD);
  double dTheta = 30 * exp(-pow(((h_ave * RAD_DEG - 275) / 25), 2));
  double R_C = 2 * sqrt(pow(C_ave, 7) / (pow(C_ave, 7) + C_25_7));
  double S_C = 1 + 0.045 * C_ave;
  double S_H = 1 + 0.015 * C_ave * T;
  double Lm50s = pow((L_ave - 50), 2);
  double S_L = 1 + 0.015 * Lm50s / sqrt(20 + Lm50s);
  double R_T = -sin(dTheta * DEG_RAD) * R_C;

  // Calculate ΔE00
  double dE_00 = sqrt(pow(dL_ / S_L, 2) + pow(dC_ / S_C, 2) + pow(dH_ / S_H, 2) + R_T * (dC_ / S_C) * (dH_ / S_H));

  return dE_00;
}


double cielab_distance(const rgb_t a, const rgb_t b) {
  xyz_t xyz_a = rgb_to_xyz(a);
  xyz_t xyz_b = rgb_to_xyz(b);

  lab_t lab_a = xyz_to_lab(xyz_a);
  lab_t lab_b = xyz_to_lab(xyz_b);

  return ciede2000(lab_a, lab_b);
}

double measure_similarity(const rgb_t a, const rgb_t b, const ALGORITHM algo) {
  switch (algo) {
  case EUCLIDIAN: return euclidian_distance(a, b);
  case WEIGHTED: return weighted_distance(a, b);
  case CIELAB: return cielab_distance(a, b);
  default:
    return -1;
  }
}

int closest_color_index(const rgb_t color, const ALGORITHM algo) {
  double smallest = 0xFFFFFFFF;
  int index = -1;

  int no_of_colors = sizeof(colors) / sizeof(*colors);

  for (int idx = 0; idx < no_of_colors; ++idx) {
    color_t diff = measure_similarity(color, colors[idx], algo);
    if (diff < smallest) {
      smallest = diff;
      index = idx;
    }
  }

  return index;
}
