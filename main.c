#include <stdio.h>
#include <stdlib.h>
#include "image_loader.h"
#include "image_analysis.h"

int main(int argc, char* argv[]) {
    // Initialize the image loading system
    if (!image_loader_init()) {
        fprintf(stderr, "Failed to initialize image loader\n");
        return 1;
    }
    
    printf("Sistema de Análise de Imagens\n");
    printf("============================\n");
    printf("%s\n\n", get_supported_formats());
    
    // Example 1: Load and analyze an image from command line argument
    if (argc > 1) {
        ImageData image;
        ImageLoadError result = load_image(argv[1], &image);
        
        if (result == IMG_SUCCESS) {
            printf("Imagem carregada com sucesso: %s\n", image.filename);
            printf("Dimensões: %dx%d pixels\n", image.width, image.height);
            printf("Canais: %d\n", image.channels);
            printf("Formato da superfície: %s\n\n", SDL_GetPixelFormatName(image.surface->format->format));
            
            // Analyze the image
            ImageAnalysis analysis;
            if (analyze_image(&image, &analysis)) {
                print_image_analysis(&analysis);
            }
            
            // Get grayscale version
            GrayscaleImage grayscale;
            if (get_grayscale_image(&image, &grayscale)) {
                print_grayscale_info(&grayscale);
                
                // Calculate and print grayscale statistics
                ImageAnalysis gray_stats;
                if (calculate_grayscale_stats(&grayscale, &gray_stats)) {
                    printf("\n=== Estatísticas da Imagem em Escala de Cinza ===\n");
                    printf("Intensidade média: %.2f\n", gray_stats.avg_intensity);
                    printf("Intensidade mínima: %d\n", gray_stats.min_intensity);
                    printf("Intensidade máxima: %d\n", gray_stats.max_intensity);
                    printf("Contraste: %d\n", gray_stats.max_intensity - gray_stats.min_intensity);
                    printf("===============================================\n");
                }
                
                // Save grayscale image
                char output_filename[256];
                if (generate_grayscale_filename(argv[1], output_filename, sizeof(output_filename))) {
                    if (save_grayscale_image(&grayscale, output_filename)) {
                        printf("\nImagem em escala de cinza salva como: %s\n", output_filename);
                    }
                }
                
                // Free grayscale image
                free_grayscale_image(&grayscale);
            }
            
            // Free the loaded image
            free_image_data(&image);
        } else {
            printf("Falha ao carregar imagem: %s\n", get_image_error_string(result));
        }
    }
    
    // Example 2: Simple image analysis testing
    const char* test_files[] = {
        "images/flowers.jpg",
        "images/bear.png",
        "images/test.png",
        "images/gray_test_image.jpeg"
    };
    
    printf("\n\nTestando analise de imagens:\n");
    printf("-----------------------------\n");
    
    int num_test_files = sizeof(test_files) / sizeof(test_files[0]);
    for (int i = 0; i < num_test_files; i++) {
        printf("\nArquivo: %s\n", test_files[i]);
        
        ImageData image;
        ImageLoadError result = load_image(test_files[i], &image);
        
        if (result == IMG_SUCCESS) {
            printf("  Carregado: %dx%d pixels, %d canais\n", 
                   image.width, image.height, image.channels);
            
            // Check if grayscale
            bool is_gray = is_image_grayscale(&image);
            printf("  Tipo: %s\n", is_gray ? "Escala de cinza" : "Colorida");
            
            // Convert to grayscale
            GrayscaleImage grayscale;
            if (get_grayscale_image(&image, &grayscale)) {
                // Calculate basic statistics
                ImageAnalysis gray_stats;
                if (calculate_grayscale_stats(&grayscale, &gray_stats)) {
                    printf("  Intensidade media: %.1f\n", gray_stats.avg_intensity);
                    printf("  Contraste: %d\n", gray_stats.max_intensity - gray_stats.min_intensity);
                }
                
                // Save grayscale image
                char output_filename[256];
                if (generate_grayscale_filename(test_files[i], output_filename, sizeof(output_filename))) {
                    if (save_grayscale_image(&grayscale, output_filename)) {
                        printf("  Salvo como: %s\n", output_filename);
                    } else {
                        printf("  Erro ao salvar imagem em escala de cinza\n");
                    }
                } else {
                    printf("  Erro ao gerar nome do arquivo de saida\n");
                }
                
                free_grayscale_image(&grayscale);
            }
            
            free_image_data(&image);
        } else {
            printf("  Erro: %s\n", get_image_error_string(result));
        }
    }
    
    // Cleanup before exit
    image_loader_cleanup();
    
    printf("\nPrograma concluído com sucesso.\n");
    return 0;
}