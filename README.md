# Índice

1. [Cripto_Steganography](#cripto_steganography)
2. [Compilación](#compilación)
3. [Ejemplos de Uso](#ejemplos-de-uso)
   - [Parámetros Generales](#parámetros-generales)
   - [Embedding](#embedding)
     - [Con cifrado](#con-cifrado)
     - [Sin cifrado](#sin-cifrado)
   - [Extraction](#extraction)
     - [Con cifrado](#con-cifrado-1)
     - [Sin cifrado](#sin-cifrado-1)
4. [Stegoanalysis](#stegoanalysis)

# Cripto_Steganography

**stegobmp** es una herramienta de esteganografía que permite ocultar y extraer información en imágenes BMP mediante técnicas de esteganografía (LSB1, LSB4, LSBI) y cifrado simétrico (AES y 3DES) con modos de operación CBC, ECB, CFB y OFB.

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
| `-p`           | Imagen portadora (archivo BMP donde se esconde o extrae la información)                         |
| `--out`         | Archivo de imagen o archivo de salida                                                           |
| `--steg`        | Algoritmo de esteganografía (`<steganography_method>`: LSB1, LSB4, LSBI)                        |
| `-a`           | Algoritmo de cifrado (`<encryption_method>`: AES128, AES192, AES256, 3DES)                      |
| `-m`           | Modo de operación de cifrado (`<mode>`: ECB, CBC, CFB, OFB)                                     |
| `--pass`        | Contraseña de cifrado (`<password>`)                                                            |

### Ejemplos de Uso


#### Embedding

##### Con cifrado

```sh
./stegobmp --embed --in <path/to/message.txt> -p <path/to/cover_image.bmp> --out <path/to/stego_image.bmp> --steg <steganography_method>
```

##### Sin cifrado

```sh
./stegobmp --embed --in <path/to/secret_data.txt> -p <path/to/cover_image.bmp> --out <path/to/stego_image_encrypted.bmp> --steg <steganography_method> -a <encryption_method> -m <mode> --pass "<password>"
```

#### Extracción

##### Con cifrado
```sh
./stegobmp --extract -p <path/to/stego_image.bmp> --out <path/to/recovered_message.txt> --steg <steganography_method>
```
##### Sin cifrado


```sh
./stegobmp --extract -p <path/to/stego_image_encrypted.bmp> --out <path/to/recovered_secret_data.txt> --steg <steganography_method> -a <encryption_method> -m <mode> --pass "<password>"
```

## Stegoanalysis

Para ver cómo se ejecuta el paso a paso para encontrar el secreto tras las imágenes en [este directorio](./assets/grupo9/), ver el README de [./stegoanalysis](./stegoanalysis).
