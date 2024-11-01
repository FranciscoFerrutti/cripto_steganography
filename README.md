# Índice

1. [Ocultamiento y Encripción de Datos en Imágenes BMP mediante Esteganografía](#ocultamiento-y-encripción-de-datos-en-imágenes-bmp-mediante-esteganografía)
2. [Compilación](#compilación)
3. [Ejemplos de Uso](#ejemplos-de-uso)
   - [Parámetros Generales](#parámetros-generales)
   - [Embedding](#embedding-en-una-imagen-bmp)
      - [Con cifrado](#con-cifrado)
      - [Sin cifrado](#sin-cifrado)
   - [Extraction](#extraction-de-una-imagen-bmp)
      - [Con cifrado](#con-cifrado-1)
      - [Sin cifrado](#sin-cifrado-1)
4. [Stegoanalysis](#stegoanalysis)

# Ocultamiento y Encripción de Datos en Imágenes BMP mediante Esteganografía

Este proyecto crea un ejecutable llamado **stegobmp** que es una herramienta de esteganografía que permite ocultar y extraer información en imágenes BMP mediante técnicas de esteganografía (LSB1, LSB4, LSBI) y cifrado simétrico (AES y 3DES) con modos de operación CBC, ECB, CFB y OFB como fué especificado en el [trabajo](./docs/Trabajo%20Practico%20Implementacion-2024.pdf). Para más detalles de implementación o técnicas ver el [análisis comparativo](./docs/Análisis_Comparativo_de_Algoritmos_de_Esteganografía_Basados_en_LSB.pdf).

## Compilación

Para compilar el proyecto, es necesario GCC y `make`. En el directorio raíz del proyecto, ejecutar:

```sh
make all

```

Esto genera un ejecutable `stegobmp`, ver los ejemplos de uso.

## Ejemplos de Uso

### Parámetros Generales

| Parámetro       | Descripción                                                                                     |
|-----------------|-------------------------------------------------------------------------------------------------|
| `--embed`       | Modo de ocultación                                                                              |
| `--extract`     | Modo de extracción                                                                              |
| `--in`          | Archivo de información a ocultar                                                                |
| `-p` o `--p`           | Imagen portadora (archivo BMP donde se esconde o extrae la información)                         |
| `--out`         | Archivo de imagen o archivo de salida                                                           |
| `--steg`        | Algoritmo de esteganografía (`<steganography_method>`: LSB1, LSB4, LSBI)                        |
| `-a` o `--a`           | Algoritmo de cifrado (`<encryption_method>`: AES128, AES192, AES256, 3DES)                      |
| `-m` o `--m`          | Modo de operación de cifrado (`<mode>`: ECB, CBC, CFB, OFB)                                     |
| `--pass`        | Contraseña de cifrado (`<password>`)                                                            |

### Ejemplos de Uso

#### Embedding en una imagen BMP

Se debe indicar como primer argumento `--embed` para indicar que se va a ocultar información en una imagen BMP. 

![embedding_example](./assets/embedding.png)

##### Sin cifrado

```sh
./stegobmp --embed --in <path/to/message.txt> -p <path/to/cover_image.bmp> --out <path/to/stego_image.bmp> --steg <steganography_method>

```

En el siguiente ejemplo se va a ocultar la imagen secreta secret_img.png en la imagen portadora porter_img.bmp con esteganografiado LSB1 y la imagen de salida que servirá como fachada para nuestros planes de destrucción será facade_img.bmp. Este es el ejemplo que se puede ver en la imagen de arriba.

```sh
./stegobmp --embed --in ./tests/secret_img.png  -p ./tests/porter_img.bmp  --out ./tests/facade_img.bmp --steg LSB1

```

##### Con cifrado

```sh
./stegobmp --embed --in <path/to/secret_data.txt> -p <path/to/cover_image.bmp> --out <path/to/stego_image_encrypted.bmp> --steg <steganography_method> -a <encryption_method> -m <mode> --pass "<password>"

```

Por ejemplo con esteganografiado LSB1 y cifrado 3DES con modo CBC

```sh
./stegobmp --embed --in ./tests/secret_img.png  -p ./tests/porter_img.bmp  --out ./tests/facade_img.bmp --steg LSB1 -a 3des -m cbc --pass "secretpassword"

```

#### Extraction de una imagen BMP

Se debe indicar como primer argumento `--extract` para indicar que se va a extraer información de una imagen BMP.

![embedding_example](./assets/extraction.png)

##### Sin cifrado

```sh
./stegobmp --extract -p <path/to/stego_image.bmp> --out <path/to/recovered_message> --steg <steganography_method>

```

Siguiendo el eje de los ejemplos de embedding, ahora la extracción se obtiene así:

```sh
./stegobmp --extract --p ./tests/facade_img.bmp --out ./tests/secret_img --steg LSB1

```

##### Con cifrado

```sh
./stegobmp --extract -p <path/to/stego_image_encrypted.bmp> --out <path/to/recovered_secret_data> --steg <steganography_method> -a <encryption_method> -m <mode> --pass "<password>"

```

Para el caso de LSB1 y cifrado 3DES con modo CBC:

```sh
./stegobmp --extract -p ./tests/facade_img.bmp --out ./tests/secret_img --steg LSB1 -a 3des -m cbc --pass "secretpassword"

```

## Stegoanalysis

```sh

```

Para ver cómo se ejecuta el paso a paso para encontrar el secreto tras las imágenes de [este directorio](./assets/grupo9/), ver el README de [./stegoanalysis](./stegoanalysis).
