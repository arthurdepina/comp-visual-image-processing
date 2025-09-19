#include "image_analysis.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

bool analyze_image(const ImageData* image_data, ImageAnalysis* analysis) {
    if (!image_data || !image_data->surface || !analysis) {
        return false;
    }
    
    SDL_Surface* surface = image_data->surface;
    
    // Initialize analysis structure
    memset(analysis, 0, sizeof(ImageAnalysis));
    analysis->width = surface->w;
    analysis->height = surface->h;
    analysis->min_intensity = 255;
    analysis->max_intensity = 0;
    
    // Determine color type based on channels
    switch (image_data->channels) {
        case 1:
            analysis->color_type = COLOR_TYPE_GRAYSCALE;
            analysis->has_transparency = false;
            break;
        case 3:
            analysis->color_type = COLOR_TYPE_RGB;
            analysis->has_transparency = false;
            break;
        case 4:
            analysis->color_type = COLOR_TYPE_RGBA;
            analysis->has_transparency = true;
            break;
        default:
            analysis->color_type = COLOR_TYPE_UNKNOWN;
            return false;
    }
    
    // Check if image is actually grayscale (even if stored as RGB)
    analysis->is_grayscale = is_image_grayscale(image_data);
    
    return true;
}

bool is_image_grayscale(const ImageData* image_data) {
    if (!image_data || !image_data->surface) {
        return false;
    }
    
    SDL_Surface* surface = image_data->surface;
    
    // If it's already single channel, it's grayscale
    if (image_data->channels == 1) {
        return true;
    }
    
    // For RGB/RGBA images, check if R == G == B for all pixels
    SDL_LockSurface(surface);
    
    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    bool is_grayscale = true;
    
    for (int y = 0; y < surface->h && is_grayscale; y++) {
        Uint8* row = pixels + y * pitch;
        
        for (int x = 0; x < surface->w && is_grayscale; x++) {
            Uint8 r, g, b;
            
            if (image_data->channels == 3) {
                // RGB format
                r = row[x * 3];
                g = row[x * 3 + 1];
                b = row[x * 3 + 2];
            } else if (image_data->channels == 4) {
                // RGBA format
                r = row[x * 4];
                g = row[x * 4 + 1];
                b = row[x * 4 + 2];
                // We ignore alpha channel for grayscale check
            } else {
                break;
            }
            
            // Check if R == G == B (with small tolerance for compression artifacts)
            if (abs(r - g) > 1 || abs(g - b) > 1 || abs(r - b) > 1) {
                is_grayscale = false;
            }
        }
    }
    
    SDL_UnlockSurface(surface);
    return is_grayscale;
}

bool convert_to_grayscale(const ImageData* image_data, GrayscaleImage* grayscale_image) {
    if (!image_data || !image_data->surface || !grayscale_image) {
        return false;
    }
    
    SDL_Surface* surface = image_data->surface;
    
    // Initialize grayscale image structure
    memset(grayscale_image, 0, sizeof(GrayscaleImage));
    grayscale_image->width = surface->w;
    grayscale_image->height = surface->h;
    grayscale_image->data_size = surface->w * surface->h;
    
    // Allocate memory for grayscale pixels
    grayscale_image->pixels = malloc(grayscale_image->data_size);
    if (!grayscale_image->pixels) {
        return false;
    }
    
    // Copy filename if available
    if (image_data->filename) {
        size_t filename_len = strlen(image_data->filename) + 1;
        grayscale_image->source_filename = malloc(filename_len);
        if (grayscale_image->source_filename) {
            strncpy(grayscale_image->source_filename, image_data->filename, filename_len);
        }
    }
    
    SDL_LockSurface(surface);
    
    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    
    // Convert using luminance formula: Y = 0.2125 * R + 0.7154 * G + 0.0721 * B
    for (int y = 0; y < surface->h; y++) {
        Uint8* row = pixels + y * pitch;
        
        for (int x = 0; x < surface->w; x++) {
            Uint8 r, g, b;
            
            if (image_data->channels == 1) {
                // Already grayscale
                grayscale_image->pixels[y * surface->w + x] = row[x];
            } else if (image_data->channels == 3) {
                // RGB format
                r = row[x * 3];
                g = row[x * 3 + 1];
                b = row[x * 3 + 2];
                
                // Apply luminance formula
                double gray = 0.2125 * r + 0.7154 * g + 0.0721 * b;
                grayscale_image->pixels[y * surface->w + x] = (Uint8)(gray + 0.5); // Round to nearest
            } else if (image_data->channels == 4) {
                // RGBA format (ignore alpha)
                r = row[x * 4];
                g = row[x * 4 + 1];
                b = row[x * 4 + 2];
                
                // Apply luminance formula
                double gray = 0.2125 * r + 0.7154 * g + 0.0721 * b;
                grayscale_image->pixels[y * surface->w + x] = (Uint8)(gray + 0.5); // Round to nearest
            }
        }
    }
    
    SDL_UnlockSurface(surface);
    
    printf("Converted to grayscale using luminance formula: Y = 0.2125*R + 0.7154*G + 0.0721*B\n");
    return true;
}

bool extract_grayscale(const ImageData* image_data, GrayscaleImage* grayscale_image) {
    if (!image_data || !image_data->surface || !grayscale_image) {
        return false;
    }
    
    // This function handles images that are already grayscale
    if (image_data->channels != 1) {
        return false; // Use convert_to_grayscale for color images
    }
    
    return convert_to_grayscale(image_data, grayscale_image);
}

bool get_grayscale_image(const ImageData* image_data, GrayscaleImage* grayscale_image) {
    if (!image_data || !grayscale_image) {
        return false;
    }
    
    // Check if already grayscale
    if (is_image_grayscale(image_data)) {
        printf("Image is already grayscale - extracting pixel data\n");
        return convert_to_grayscale(image_data, grayscale_image);
    } else {
        printf("Image contains colors - converting to grayscale\n");
        return convert_to_grayscale(image_data, grayscale_image);
    }
}

Uint8 get_grayscale_pixel(const GrayscaleImage* grayscale_image, int x, int y) {
    if (!grayscale_image || !grayscale_image->pixels) {
        return 0;
    }
    
    if (x < 0 || x >= grayscale_image->width || y < 0 || y >= grayscale_image->height) {
        return 0;
    }
    
    return grayscale_image->pixels[y * grayscale_image->width + x];
}

bool set_grayscale_pixel(GrayscaleImage* grayscale_image, int x, int y, Uint8 value) {
    if (!grayscale_image || !grayscale_image->pixels) {
        return false;
    }
    
    if (x < 0 || x >= grayscale_image->width || y < 0 || y >= grayscale_image->height) {
        return false;
    }
    
    grayscale_image->pixels[y * grayscale_image->width + x] = value;
    return true;
}

bool calculate_grayscale_stats(const GrayscaleImage* grayscale_image, ImageAnalysis* analysis) {
    if (!grayscale_image || !grayscale_image->pixels || !analysis) {
        return false;
    }
    
    analysis->width = grayscale_image->width;
    analysis->height = grayscale_image->height;
    analysis->color_type = COLOR_TYPE_GRAYSCALE;
    analysis->is_grayscale = true;
    analysis->has_transparency = false;
    analysis->min_intensity = 255;
    analysis->max_intensity = 0;
    
    long long sum = 0;
    size_t total_pixels = grayscale_image->width * grayscale_image->height;
    
    for (size_t i = 0; i < total_pixels; i++) {
        Uint8 pixel = grayscale_image->pixels[i];
        sum += pixel;
        
        if (pixel < analysis->min_intensity) {
            analysis->min_intensity = pixel;
        }
        if (pixel > analysis->max_intensity) {
            analysis->max_intensity = pixel;
        }
    }
    
    analysis->avg_intensity = (double)sum / total_pixels;
    return true;
}

void print_image_analysis(const ImageAnalysis* analysis) {
    if (!analysis) {
        return;
    }
    
    printf("\n=== Análise da Imagem ===\n");
    printf("Dimensões: %dx%d pixels\n", analysis->width, analysis->height);
    printf("Tipo de cor: %s\n", get_color_type_string(analysis->color_type));
    printf("É escala de cinza: %s\n", analysis->is_grayscale ? "Sim" : "Não");
    printf("Tem transparência: %s\n", analysis->has_transparency ? "Sim" : "Não");
    printf("Intensidade média: %.2f\n", analysis->avg_intensity);
    printf("Intensidade mínima: %d\n", analysis->min_intensity);
    printf("Intensidade máxima: %d\n", analysis->max_intensity);
    printf("========================\n");
}

void print_grayscale_info(const GrayscaleImage* grayscale_image) {
    if (!grayscale_image) {
        return;
    }
    
    printf("\n=== Imagem em Escala de Cinza ===\n");
    printf("Dimensões: %dx%d pixels\n", grayscale_image->width, grayscale_image->height);
    printf("Tamanho dos dados: %zu bytes\n", grayscale_image->data_size);
    if (grayscale_image->source_filename) {
        printf("Arquivo fonte: %s\n", grayscale_image->source_filename);
    }
    printf("================================\n");
}

bool save_grayscale_image(const GrayscaleImage* grayscale_image, const char* output_path) {
    if (!grayscale_image || !grayscale_image->pixels || !output_path) {
        return false;
    }
    
    // Create SDL surface from grayscale data
    SDL_Surface* surface = SDL_CreateRGBSurface(0, 
                                               grayscale_image->width, 
                                               grayscale_image->height, 
                                               24, // 24-bit RGB
                                               0x000000FF, // R mask
                                               0x0000FF00, // G mask  
                                               0x00FF0000, // B mask
                                               0);         // No alpha
    
    if (!surface) {
        printf("Erro ao criar superficie SDL: %s\n", SDL_GetError());
        return false;
    }
    
    SDL_LockSurface(surface);
    
    // Convert grayscale to RGB (R=G=B for each pixel)
    Uint8* pixels = (Uint8*)surface->pixels;
    int pitch = surface->pitch;
    
    for (int y = 0; y < grayscale_image->height; y++) {
        Uint8* row = pixels + y * pitch;
        
        for (int x = 0; x < grayscale_image->width; x++) {
            Uint8 gray_value = grayscale_image->pixels[y * grayscale_image->width + x];
            
            // Set R, G, B to the same grayscale value
            row[x * 3] = gray_value;     // R
            row[x * 3 + 1] = gray_value; // G
            row[x * 3 + 2] = gray_value; // B
        }
    }
    
    SDL_UnlockSurface(surface);
    
    // Save as PNG
    int result = IMG_SavePNG(surface, output_path);
    
    SDL_FreeSurface(surface);
    
    if (result == 0) {
        printf("Imagem em escala de cinza salva: %s\n", output_path);
        return true;
    } else {
        printf("Erro ao salvar imagem: %s\n", IMG_GetError());
        return false;
    }
}

bool generate_grayscale_filename(const char* original_filename, char* output_buffer, size_t buffer_size) {
    if (!original_filename || !output_buffer || buffer_size == 0) {
        return false;
    }
    
    // Find the last occurrence of '/' to get just the filename
    const char* filename = strrchr(original_filename, '/');
    if (filename) {
        filename++; // Skip the '/'
    } else {
        filename = original_filename; // No path separator found
    }
    
    // Find the last occurrence of '.' to remove extension
    const char* extension = strrchr(filename, '.');
    size_t name_length;
    
    if (extension) {
        name_length = extension - filename;
    } else {
        name_length = strlen(filename);
    }
    
    // Generate new filename: grayscale_images/original_name_gray.png
    int written = snprintf(output_buffer, buffer_size, "grayscale_images/%.*s_gray.png", 
                          (int)name_length, filename);
    
    if (written < 0 || (size_t)written >= buffer_size) {
        return false; // Buffer too small or error
    }
    
    return true;
}

void free_grayscale_image(GrayscaleImage* grayscale_image) {
    if (!grayscale_image) {
        return;
    }
    
    if (grayscale_image->pixels) {
        free(grayscale_image->pixels);
        grayscale_image->pixels = NULL;
    }
    
    if (grayscale_image->source_filename) {
        free(grayscale_image->source_filename);
        grayscale_image->source_filename = NULL;
    }
    
    grayscale_image->width = 0;
    grayscale_image->height = 0;
    grayscale_image->data_size = 0;
}

const char* get_color_type_string(ColorType color_type) {
    switch (color_type) {
        case COLOR_TYPE_GRAYSCALE:
            return "Escala de Cinza (1 canal)";
        case COLOR_TYPE_RGB:
            return "RGB (3 canais)";
        case COLOR_TYPE_RGBA:
            return "RGBA (4 canais)";
        case COLOR_TYPE_UNKNOWN:
        default:
            return "Desconhecido";
    }
}