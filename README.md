# Computação Visual -- Processamento de Imagens

A modular computer vision system built in C using SDL2 for image processing.

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

## Sistema de Carregamento de Imagens

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