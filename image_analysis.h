#ifndef IMAGE_ANALYSIS_H
#define IMAGE_ANALYSIS_H

#include <SDL2/SDL.h>
#include <stdbool.h>
#include "image_loader.h"

// Color type classification
typedef enum {
    COLOR_TYPE_GRAYSCALE = 1,
    COLOR_TYPE_RGB = 3,
    COLOR_TYPE_RGBA = 4,
    COLOR_TYPE_UNKNOWN = 0
} ColorType;

// Structure to hold analysis results
typedef struct {
    ColorType color_type;
    bool is_grayscale;
    bool has_transparency;
    int width;
    int height;
    double avg_intensity;
    int min_intensity;
    int max_intensity;
} ImageAnalysis;

// Structure to hold grayscale image data
typedef struct {
    Uint8* pixels;          // Grayscale pixel data (0-255)
    int width;
    int height;
    size_t data_size;       // Total bytes allocated
    char* source_filename;  // Original image filename
} GrayscaleImage;

/**
 * Analyze an image to determine its color properties
 * @param image_data Loaded image data
 * @param analysis Pointer to store analysis results
 * @return true on success, false on failure
 */
bool analyze_image(const ImageData* image_data, ImageAnalysis* analysis);

/**
 * Check if an image is already in grayscale
 * This function analyzes pixel data to determine if the image contains colors
 * @param image_data Loaded image data
 * @return true if image is grayscale, false if it contains colors
 */
bool is_image_grayscale(const ImageData* image_data);

/**
 * Convert a color image to grayscale using luminance formula
 * Formula: Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
 * @param image_data Source image data
 * @param grayscale_image Pointer to store grayscale result
 * @return true on success, false on failure
 */
bool convert_to_grayscale(const ImageData* image_data, GrayscaleImage* grayscale_image);

/**
 * Create a grayscale image from already grayscale ImageData
 * @param image_data Source grayscale image data
 * @param grayscale_image Pointer to store grayscale result
 * @return true on success, false on failure
 */
bool extract_grayscale(const ImageData* image_data, GrayscaleImage* grayscale_image);

/**
 * Get grayscale image (convert if needed, extract if already grayscale)
 * This is the main function to use - handles both cases automatically
 * @param image_data Source image data
 * @param grayscale_image Pointer to store grayscale result
 * @return true on success, false on failure
 */
bool get_grayscale_image(const ImageData* image_data, GrayscaleImage* grayscale_image);

/**
 * Get pixel value at specific coordinates in grayscale image
 * @param grayscale_image Grayscale image data
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to height-1)
 * @return Pixel value (0-255) or 0 if coordinates are invalid
 */
Uint8 get_grayscale_pixel(const GrayscaleImage* grayscale_image, int x, int y);

/**
 * Set pixel value at specific coordinates in grayscale image
 * @param grayscale_image Grayscale image data
 * @param x X coordinate (0 to width-1)
 * @param y Y coordinate (0 to height-1)
 * @param value Pixel value (0-255)
 * @return true on success, false if coordinates are invalid
 */
bool set_grayscale_pixel(GrayscaleImage* grayscale_image, int x, int y, Uint8 value);

/**
 * Calculate basic statistics for grayscale image
 * @param grayscale_image Grayscale image data
 * @param analysis Pointer to store statistical results
 * @return true on success, false on failure
 */
bool calculate_grayscale_stats(const GrayscaleImage* grayscale_image, ImageAnalysis* analysis);

/**
 * Print detailed analysis information
 * @param analysis Analysis results to print
 */
void print_image_analysis(const ImageAnalysis* analysis);

/**
 * Print grayscale image information
 * @param grayscale_image Grayscale image to describe
 */
void print_grayscale_info(const GrayscaleImage* grayscale_image);

/**
 * Save grayscale image to file as PNG
 * @param grayscale_image Grayscale image to save
 * @param output_path Path where to save the image
 * @return true on success, false on failure
 */
bool save_grayscale_image(const GrayscaleImage* grayscale_image, const char* output_path);

/**
 * Generate output filename for grayscale image
 * Converts "images/flowers.jpg" to "grayscale_images/flowers_gray.png"
 * @param original_filename Original image filename
 * @param output_buffer Buffer to store the generated filename
 * @param buffer_size Size of the output buffer
 * @return true on success, false on failure
 */
bool generate_grayscale_filename(const char* original_filename, char* output_buffer, size_t buffer_size);

/**
 * Free memory allocated for grayscale image
 * @param grayscale_image Grayscale image to free
 */
void free_grayscale_image(GrayscaleImage* grayscale_image);

/**
 * Get color type as string
 * @param color_type Color type enum value
 * @return String description of color type
 */
const char* get_color_type_string(ColorType color_type);

#endif // IMAGE_ANALYSIS_H