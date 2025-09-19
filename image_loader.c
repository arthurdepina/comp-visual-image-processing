#include "image_loader.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Global flag to track initialization status
static bool g_initialized = false;

bool image_loader_init(void) {
    if (g_initialized) {
        return true;
    }
    
    // Initialize SDL if not already done
    if (SDL_Init(SDL_INIT_VIDEO) < 0) {
        fprintf(stderr, "SDL could not initialize! SDL Error: %s\n", SDL_GetError());
        return false;
    }
    
    // Initialize SDL_image with support for PNG, JPG, and BMP
    int img_flags = IMG_INIT_PNG | IMG_INIT_JPG;
    if (!(IMG_Init(img_flags) & img_flags)) {
        fprintf(stderr, "SDL_image could not initialize! SDL_image Error: %s\n", IMG_GetError());
        SDL_Quit();
        return false;
    }
    
    g_initialized = true;
    printf("Image loader initialized successfully\n");
    return true;
}

ImageLoadError load_image(const char* filename, ImageData* image_data) {
    if (!g_initialized) {
        fprintf(stderr, "Image loader not initialized! Call image_loader_init() first.\n");
        return IMG_ERROR_SDL_NOT_INITIALIZED;
    }
    
    if (!filename || !image_data) {
        fprintf(stderr, "Invalid parameters passed to load_image\n");
        return IMG_ERROR_UNKNOWN;
    }
    
    // Initialize the image_data structure
    memset(image_data, 0, sizeof(ImageData));
    
    // Check if file exists
    if (!file_exists(filename)) {
        fprintf(stderr, "File not found: %s\n", filename);
        return IMG_ERROR_FILE_NOT_FOUND;
    }
    
    // Load the image
    SDL_Surface* loaded_surface = IMG_Load(filename);
    if (!loaded_surface) {
        fprintf(stderr, "Unable to load image %s! SDL_image Error: %s\n", 
                filename, IMG_GetError());
        
        // Try to determine the specific error type
        const char* error = IMG_GetError();
        if (strstr(error, "Unsupported image format") || 
            strstr(error, "not a") || 
            strstr(error, "Invalid")) {
            return IMG_ERROR_INVALID_FORMAT;
        }
        return IMG_ERROR_UNKNOWN;
    }
    
    // Store image information
    image_data->surface = loaded_surface;
    image_data->width = loaded_surface->w;
    image_data->height = loaded_surface->h;
    image_data->channels = loaded_surface->format->BytesPerPixel;
    
    // Store filename (make a copy)
    size_t filename_len = strlen(filename) + 1;
    image_data->filename = malloc(filename_len);
    if (image_data->filename) {
        strncpy(image_data->filename, filename, filename_len);
    }
    
    printf("Image loaded successfully: %s (%dx%d, %d channels)\n", 
           filename, image_data->width, image_data->height, image_data->channels);
    
    return IMG_SUCCESS;
}

void free_image_data(ImageData* image_data) {
    if (!image_data) {
        return;
    }
    
    if (image_data->surface) {
        SDL_FreeSurface(image_data->surface);
        image_data->surface = NULL;
    }
    
    if (image_data->filename) {
        free(image_data->filename);
        image_data->filename = NULL;
    }
    
    // Reset other fields
    image_data->width = 0;
    image_data->height = 0;
    image_data->channels = 0;
}

const char* get_image_error_string(ImageLoadError error) {
    switch (error) {
        case IMG_SUCCESS:
            return "Success";
        case IMG_ERROR_FILE_NOT_FOUND:
            return "File not found or not accessible";
        case IMG_ERROR_INVALID_FORMAT:
            return "Invalid or unsupported image format";
        case IMG_ERROR_SDL_NOT_INITIALIZED:
            return "SDL/SDL_image not initialized";
        case IMG_ERROR_MEMORY_ALLOCATION:
            return "Memory allocation failed";
        case IMG_ERROR_UNKNOWN:
        default:
            return "Unknown error occurred";
    }
}

bool file_exists(const char* filename) {
    if (!filename) {
        return false;
    }
    
    FILE* file = fopen(filename, "rb");
    if (file) {
        fclose(file);
        return true;
    }
    return false;
}

const char* get_supported_formats(void) {
    return "Supported formats: PNG, JPG, JPEG, BMP, GIF, TIF, TIFF";
}

void image_loader_cleanup(void) {
    if (!g_initialized) {
        return;
    }
    
    IMG_Quit();
    SDL_Quit();
    g_initialized = false;
    printf("Image loader cleaned up\n");
}