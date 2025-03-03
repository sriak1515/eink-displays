# cython: language_level=3
# cython: boundscheck=False
# cython: wraparound=False

import numpy as np
cimport numpy as cnp

cdef extern from "numpy/ndarrayobject.h":
    void import_array()

# Make sure to define the import_array function to prevent Cython from throwing an error
def init_numpy():
    import_array()

# Function to convert RGB to CIELAB
cdef cnp.ndarray[double, ndim=1] rgb_to_cielab(cnp.ndarray[cnp.uint8_t, ndim=1] rgb):
    cdef double dR = rgb[0] / 255.0
    cdef double dG = rgb[1] / 255.0
    cdef double dB = rgb[2] / 255.0
    
    cdef double r = dR / 12.92 if dR <= 0.04045 else ((dR + 0.055) / 1.055) ** 2.4
    cdef double g = dG / 12.92 if dG <= 0.04045 else ((dG + 0.055) / 1.055) ** 2.4
    cdef double b = dB / 12.92 if dB <= 0.04045 else ((dB + 0.055) / 1.055) ** 2.4
    
    cdef double X = r * 0.4124564 + g * 0.3575761 + b * 0.1804375
    cdef double Y = r * 0.2126729 + g * 0.7151522 + b * 0.0721750
    cdef double Z = r * 0.0193339 + g * 0.1191920 + b * 0.9503041
    
    cdef double epsilon = 0.008856
    cdef double kappa = 903.3
    cdef double Xr = 0.950456
    cdef double Yr = 1.0
    cdef double Zr = 1.088754
    
    cdef double xr = X / Xr
    cdef double yr = Y / Yr
    cdef double zr = Z / Zr
    
    cdef double fx = pow(xr, 1.0 / 3.0) if xr > epsilon else (kappa * xr + 16.0) / 116.0
    cdef double fy = pow(yr, 1.0 / 3.0) if yr > epsilon else (kappa * yr + 16.0) / 116.0
    cdef double fz = pow(zr, 1.0 / 3.0) if zr > epsilon else (kappa * zr + 16.0) / 116.0
    
    cdef double L = 116.0 * fy - 16.0
    cdef double A = 500.0 * (fx - fy)
    cdef double B = 200.0 * (fy - fz)
    
    cdef cnp.ndarray[double, ndim=1] lab = np.zeros((3), dtype=np.double)
    lab[0] = L
    lab[1] = A
    lab[2] = B
    return lab

def floyd_steinberg(cnp.ndarray[cnp.uint8_t, ndim=3] input_img, cnp.ndarray[cnp.uint8_t, ndim=2] epd_colors, double dithering_strength, int EPD_W, int EPD_H, int enable_lab = 0):
    """
    Apply Floyd-Steinberg dithering with strength control to an input image using color theory.
    
    Parameters:
        input_img (numpy.ndarray): Input image with shape (height, width, 3) of type uint8.
        epd_colors (numpy.ndarray): Array of EPD colors with shape (num_colors, 3) of type double.
        dithering_strength (double): Dithering strength parameter from 0.0 to 1.0.
        EPD_W (int): Width of the EPD display.
        EPD_H (int): Height of the EPD display.
        
    Returns:
        output_img (numpy.ndarray): Output dithered image with shape (height, width, 3) of type uint8.
    """
    cdef int height = input_img.shape[0]
    cdef int width = input_img.shape[1]
    cdef int num_colors = epd_colors.shape[0]
    cdef int x, y, c, best
    cdef double min_diff, diff, scaled_diff
    cdef cnp.ndarray[double, ndim=2] epd_colors_lab = np.zeros((num_colors, 3), dtype=np.double)

    for color in range(num_colors):
        epd_colors_lab[color] = rgb_to_cielab(epd_colors[color])
    cdef cnp.ndarray[double, ndim=1] epd_color 
    cdef cnp.ndarray[double, ndim=1] pixel_lab
    cdef cnp.ndarray[double, ndim=1] epd_color_lab
    cdef cnp.ndarray[cnp.uint8_t, ndim=3] output_img = np.zeros((height, width, 3), dtype=np.uint8)
    
    for y in range(height):
        for x in range(width):
            # Convert current pixel to CIELAB
            pixel_lab = rgb_to_cielab(input_img[y, x])
            
            # Find the best matching color based on delta E^2
            min_diff = 1e10
            best = 0
            for c in range(num_colors):
                if enable_lab > 0:
                    epd_color_lab = epd_colors_lab[c]
                    diff = (pixel_lab[0] - epd_color_lab[0]) ** 2 + \
                           (pixel_lab[1] - epd_color_lab[1]) ** 2 + \
                           (pixel_lab[2] - epd_color_lab[2]) ** 2
                else:
                    epd_color = epd_colors[c]/255.0
                    diff = (input_img[y, x, 0]/255.0 - epd_color[0]) ** 2 + \
                           (input_img[y, x, 1]/255.0 - epd_color[1]) ** 2 + \
                           (input_img[y, x, 2]/255.0 - epd_color[2]) ** 2
                if diff < min_diff:
                    min_diff = diff
                    best = c
            
            # Floyd-Steinberg error distribution with strength control
            diff = 0.0
            scaled_diff = 0.0
            
            for c in range(3):
                diff = input_img[y, x, c] / 255.0 - epd_colors[best, c] / 255.0
                scaled_diff = diff * dithering_strength
            
                # Right pixel
                if x + 1 < EPD_W:
                    input_img[y, x + 1, c] = <cnp.uint8_t>(min(max(input_img[y, x + 1, c] + <int>(scaled_diff * 7/16 * 255), 0), 255))
                
                # Bottom-left pixel
                if x - 1 >= 0 and y + 1 < EPD_H:
                    input_img[y + 1, x - 1, c] = <cnp.uint8_t>(min(max(input_img[y + 1, x - 1, c] + <int>(scaled_diff * 3/16 * 255), 0), 255))
                
                # Bottom pixel
                if y + 1 < EPD_H:
                    input_img[y + 1, x, c] = <cnp.uint8_t>(min(max(input_img[y + 1, x, c] + <int>(scaled_diff * 5/16 * 255), 0), 255))
                
                # Bottom-right pixel
                if x + 1 < EPD_W and y + 1 < EPD_H:
                    input_img[y + 1, x + 1, c] = <cnp.uint8_t>(min(max(input_img[y + 1, x + 1, c] + <int>(scaled_diff * 1/16 * 255), 0), 255))

            # Set output image pixel
            for c in range(3):
                output_img[y, x, c] = <cnp.uint8_t>(epd_colors[best, c])
    
    return output_img