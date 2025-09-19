#ifndef IMAGE_LOADER_H
#define IMAGE_LOADER_H

#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <stdbool.h>

// Error codes for image loading operations
typedef enum {
    IMG_SUCCESS = 0,
    IMG_ERROR_FILE_NOT_FOUND,
    IMG_ERROR_INVALID_FORMAT,
    IMG_ERROR_SDL_NOT_INITIALIZED,
    IMG_ERROR_MEMORY_ALLOCATION,
    IMG_ERROR_UNKNOWN
} ImageLoadError;

// Structure to hold image data and metadata
typedef struct {
    SDL_Surface* surface;
    int width;
    int height;
    int channels;
    char* filename;
} ImageData;

/**
 * Initialize the image loading system
 * Must be called before using any other image loading functions
 * @return true on success, false on failure
 */
bool image_loader_init(void);

/**
 * Load an image from file
 * Supports PNG, JPG, JPEG, BMP, GIF, TIF, TIFF formats
 * @param filename Path to the image file
 * @param image_data Pointer to ImageData structure to store the loaded image
 * @return ImageLoadError code indicating success or type of error
 */
ImageLoadError load_image(const char* filename, ImageData* image_data);

/**
 * Free memory allocated for an ImageData structure
 * @param image_data Pointer to ImageData structure to free
 */
void free_image_data(ImageData* image_data);

/**
 * Get a human-readable error message for an ImageLoadError code
 * @param error The error code
 * @return String describing the error
 */
const char* get_image_error_string(ImageLoadError error);

/**
 * Check if a file exists and is readable
 * @param filename Path to check
 * @return true if file exists and is readable, false otherwise
 */
bool file_exists(const char* filename);

/**
 * Get supported image formats as a string
 * @return String listing supported formats
 */
const char* get_supported_formats(void);

/**
 * Cleanup the image loading system
 * Should be called before program exit
 */
void image_loader_cleanup(void);

#endif // IMAGE_LOADER_H