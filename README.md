# Computação Visual - Processamento de Imagens

## Quick Start

### Build and Run
```bash
# Instalar dependências (macOS)
make install-deps-macos

# Compilar o projeto
make

# Testar com sua própria imagem
./bin/image_loader_demo images/sua_imagem.png

# Executar testes integrados
make test
```

## Uso Básico
```bash
# Carregar e exibir informações sobre uma imagem
./bin/image_loader_demo caminho/para/imagem.jpg

# Executar sem argumentos para ver formatos suportados e testar vários arquivos
./bin/image_loader_demo
```

# Parte 1: Sistema de Carregamento de Imagens

A funcionalidade de carregamento de imagens é implementada como um módulo separado (`image_loader.c` e `image_loader.h`) que fornece uma API limpa para carregar e gerenciar imagens em aplicações de visão computacional.

### Arquitetura do Sistema

**Separação de Responsabilidades**: O módulo é completamente independente do código principal, seguindo princípios de programação modular. Isso permite reutilização em diferentes contextos de visão computacional sem modificações.

**Gerenciamento de Estado Global**: O sistema mantém um estado global de inicialização (`g_initialized`) para garantir que SDL2 seja configurado apenas uma vez e prevenir múltiplas inicializações.

**Abstração de Complexidade**: A API esconde a complexidade do SDL2/SDL2_image, oferecendo uma interface simplificada para operações comuns de carregamento de imagem.

### Estruturas de Dados

```c
typedef struct {
    SDL_Surface* surface;    // Superfície SDL com dados de pixel
    int width;              // Largura em pixels
    int height;             // Altura em pixels  
    int channels;           // Canais de cor (1-4)
    char* filename;         // Nome do arquivo original
} ImageData;
```

**SDL_Surface**: Contém os dados brutos dos pixels, informações de formato (RGB, RGBA, etc.), e metadados sobre organização da memória (pitch, máscaras de bits).

**Gerenciamento de Memória**: Cada `ImageData` aloca memória dinamicamente para o filename e referencia uma SDL_Surface que deve ser liberada apropriadamente.

### Códigos de Erro Estruturados

```c
typedef enum {
    IMG_SUCCESS = 0,                    // Operação bem-sucedida
    IMG_ERROR_FILE_NOT_FOUND,          // Arquivo inexistente ou inacessível
    IMG_ERROR_INVALID_FORMAT,          // Formato não suportado ou corrupto
    IMG_ERROR_SDL_NOT_INITIALIZED,     // SDL não foi inicializado
    IMG_ERROR_MEMORY_ALLOCATION,       // Falha na alocação de memória
    IMG_ERROR_UNKNOWN                  // Erro não categorizado
} ImageLoadError;
```

### Formatos Suportados e Detecção

- **PNG**: Suporte completo via libpng, incluindo transparência e compressão
- **JPG/JPEG**: Usando libjpeg, com suporte a diferentes qualidades de compressão
- **BMP**: Formato nativo do Windows, múltiplas profundidades de bit
- **GIF**: Incluindo animações (apenas primeiro frame é carregado)
- **TIF/TIFF**: Formatos de alta qualidade, múltiplas camadas

**Detecção Automática**: SDL2_image analisa os magic bytes do arquivo para determinar o formato automaticamente, independente da extensão.

### Fluxo de Carregamento Detalhado

#### 1. Inicialização (`image_loader_init()`)
```c
// Inicializa SDL_VIDEO (necessário para superfícies)
SDL_Init(SDL_INIT_VIDEO)

// Carrega bibliotecas de formato específicas
IMG_Init(IMG_INIT_PNG | IMG_INIT_JPG)
```

#### 2. Validação Pré-carregamento
- **Verificação de Parâmetros**: Ponteiros nulos, strings vazias
- **Teste de Existência**: `fopen()` em modo binário para verificar acessibilidade
- **Verificação de Inicialização**: Estado global do sistema

#### 3. Carregamento de Dados (`IMG_Load()`)
```c
SDL_Surface* surface = IMG_Load(filename);
```
**Processo Interno**:
- Leitura e análise do cabeçalho do arquivo
- Determinação do formato baseada em magic bytes
- Decodificação usando biblioteca apropriada (libpng, libjpeg, etc.)
- Criação de SDL_Surface com layout de pixel otimizado

#### 4. Extração de Metadados
```c
image_data->width = surface->w;
image_data->height = surface->h;
image_data->channels = surface->format->BytesPerPixel;
```

**Informações de Formato**:
- **BytesPerPixel**: Número de bytes por pixel (1-4)
- **Format**: Estrutura detalhada com máscaras RGB, ordem de bytes
- **Pitch**: Bytes por linha (pode incluir padding para alinhamento)

#### 5. Tratamento de Erros Específicos
```c
// Análise da mensagem de erro SDL para categorização
const char* error = IMG_GetError();
if (strstr(error, "Unsupported image format")) {
    return IMG_ERROR_INVALID_FORMAT;
}
```

### Gerenciamento de Memória

**Alocação**:
- SDL_Surface aloca internamente buffer de pixels
- Filename é copiado dinamicamente com `malloc()`
- Estrutura ImageData é gerenciada pelo usuário

**Liberação**:
```c
void free_image_data(ImageData* image_data) {
    SDL_FreeSurface(image_data->surface);  // Libera buffer de pixels
    free(image_data->filename);            // Libera string do filename
    // Zera todos os campos para prevenir uso após liberação
}
```

### Considerações de Performance

**Carregamento Lazy**: Imagens são carregadas apenas quando solicitadas, não em lote.

**Formato de Superfície**: SDL2 pode converter automaticamente para formato otimizado para a plataforma de destino.

**Validação Prévia**: Verificação de arquivo antes do carregamento evita tentativas custosas em arquivos inválidos.

### Integração com Visão Computacional

**Acesso a Pixels**: SDL_Surface fornece acesso direto ao buffer de pixels via `surface->pixels`

**Múltiplos Formatos de Pixel**: Suporte automático para:
- Escala de cinza (8-bit)
- RGB (24-bit)  
- RGBA (32-bit)
- Formatos indexed color

**Compatibilidade**: Dados podem ser facilmente convertidos para bibliotecas como OpenCV, PIL, ou processamento manual de pixels.

A API é projetada para ser simples, mas robusta - carregue uma imagem com uma única chamada de função, obtenha informações detalhadas de erro se algo der errado, e acesse facilmente todas as propriedades da imagem para processamento posterior.

# Parte 2: Análise e Conversão para Escala de Cinza

O sistema inclui um módulo completo de análise de imagens (`image_analysis.c` e `image_analysis.h`) que fornece detecção automática de tipo de cor, conversão para escala de cinza e análise estatística para aplicações de visão computacional.

### Detecção Automática de Tipo de Imagem

**Classificação por Canais**: O sistema classifica automaticamente imagens baseado no número de canais:
- **1 canal**: Escala de cinza nativa
- **3 canais**: RGB sem transparência
- **4 canais**: RGBA com canal alpha

**Detecção Inteligente de Escala de Cinza**: Mesmo imagens armazenadas como RGB podem ser detectadas como escala de cinza se todos os pixels possuem valores R=G=B. O algoritmo verifica pixel por pixel com tolerância de ±1 para artefatos de compressão:

```c
// Verificação com tolerância para artefatos de compressão
if (abs(r - g) > 1 || abs(g - b) > 1 || abs(r - b) > 1) {
    is_grayscale = false;
}
```

### Conversão para Escala de Cinza

**Fórmula de Luminância**: Para imagens coloridas, utiliza-se a fórmula padrão ITU-R BT.709 que pondera os canais RGB baseado na sensibilidade do olho humano:

```
Y = 0.2125 × R + 0.7154 × G + 0.0721 × B
```

**Justificativa Técnica**:
- **Verde (0.7154)**: Maior peso devido à alta sensibilidade do olho humano ao verde
- **Vermelho (0.2125)**: Peso médio, segunda maior sensibilidade
- **Azul (0.0721)**: Menor peso, menor sensibilidade ocular

**Processo de Conversão**:
1. **Análise Prévia**: Determina se conversão ou extração é necessária
2. **Acesso Seguro aos Pixels**: Utiliza `SDL_LockSurface()` para acesso thread-safe
3. **Cálculo de Luminância**: Aplica a fórmula com arredondamento para o inteiro mais próximo
4. **Alocação de Memória**: Cria buffer contíguo para dados em escala de cinza

### Estruturas de Dados Especializadas

```c
typedef struct {
    Uint8* pixels;          // Buffer linear de pixels (0-255)
    int width, height;      // Dimensões da imagem
    size_t data_size;       // Tamanho total em bytes
    char* source_filename;  // Arquivo fonte original
} GrayscaleImage;
```

**Organização de Memória**: Os pixels são armazenados em formato linear (row-major order) para otimização de cache e acesso sequencial eficiente.

### Análise Estatística

**Métricas Calculadas**:
- **Intensidade Média**: Média aritmética de todos os pixels, indica brilho geral
- **Intensidade Mínima/Máxima**: Range dinâmico da imagem
- **Contraste**: Diferença entre intensidade máxima e mínima

**Algoritmo de Cálculo**:
```c
// Cálculo em uma única passada para eficiência
for (size_t i = 0; i < total_pixels; i++) {
    Uint8 pixel = grayscale_image->pixels[i];
    sum += pixel;
    min_intensity = (pixel < min_intensity) ? pixel : min_intensity;
    max_intensity = (pixel > max_intensity) ? pixel : max_intensity;
}
avg_intensity = (double)sum / total_pixels;
```

### Persistência de Dados

**Salvamento em PNG**: As imagens em escala de cinza são salvas como PNG RGB onde R=G=B para cada pixel, garantindo compatibilidade universal:

```c
// Conversão para RGB para salvamento
row[x * 3] = gray_value;     // R
row[x * 3 + 1] = gray_value; // G  
row[x * 3 + 2] = gray_value; // B
```

**Geração Automática de Nomes**: Converte automaticamente nomes de arquivos:
- `images/flowers.jpg` → `grayscale_images/flowers_gray.png`
- Preserva o nome base e adiciona sufixo `_gray`

### Acesso e Manipulação de Pixels

**Acesso Seguro**: Funções `get_grayscale_pixel()` e `set_grayscale_pixel()` com verificação de limites automática:

```c
// Fórmula de acesso linear
pixel_index = y * width + x;
```

**Verificação de Limites**: Todas as operações verificam coordenadas válidas (0 ≤ x < width, 0 ≤ y < height) para prevenir acessos inválidos à memória.

### Otimizações de Performance

- **Acesso Linear**: Dados organizados sequencialmente para otimização de cache
- **Cálculo Único**: Estatísticas calculadas em uma única passada pelos dados
- **Gerenciamento Eficiente**: Funções dedicadas para alocação e liberação de memória
- **Conversão In-Place**: Quando possível, reutiliza estruturas existentes

O módulo serve como base sólida para operações avançadas de visão computacional, fornecendo dados em escala de cinza normalizados e estatísticas essenciais para algoritmos subsequentes.
