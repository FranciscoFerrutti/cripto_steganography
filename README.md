# cripto_steganography
stegoBMP es una herramienta de esteganografía que permite ocultar información en imágenes BMP. La herramienta permite ocultar información en imágenes BMP utilizando el algoritmo de esteganografía LSB1 y cifrado de la información utilizando algoritmos de cifrado simétrico como AES y 3DES con modos de operación CBC, ECB, CFB y OFB. La herramienta también permite extraer la información oculta en una imagen BMP.

## Compilar

Para compilar el proyecto se necesita tener instalado el compilador GCC y la herramienta de compilación make. Para compilar el proyecto se debe ejecutar el siguiente comando en el directorio raíz del proyecto.

```sh {"id":"01JA3VGPSHQWK1WT026B2474MV"}
make all
```

## Ejemplos de uso

### Ocultar información en una imagen BMP sin cifrado

Se debe indicar como primer argumento `--embed` para indicar que se va a ocultar información en una imagen BMP. Los siguientes argumentos pueden variar su orden:
* `--in` para indicar la ruta de la imagen que se va a ocultar.
* `--p` para indicar la ruta de la imagen que servirá como portadora.
* `--out` para indicar la ruta de la imagen de salida.
* `--steg` para indicar el algoritmo de esteganografía a utilizar.

En el siguiente ejemplo se va a ocultar la imagen secreta `secret_img.bmp` en la imagen portadora `porter_img.bmp` con esteganografiado LSB1 y la imagen de salida que servirá como fachada para nuestros planes de destrucción será `facade_img.bmp`.

```sh {"id":"01JA3VH2E57SH82R7WBAG1AJ72"}
./stegobmp --embed --in ./tests/secret_img.bmp  --p ./tests/porter_img.bmp  --out ./tests/facade_img.bmp --steg LSB1
```

### Ocultar información en una imagen BMP con cifrado

Se debe indicar como primer argumento `--embed` para indicar que se va a ocultar información en una imagen BMP. Los siguientes argumentos pueden variar su orden:
* `--in` para indicar la ruta de la imagen que se va a ocultar.
* `--p` para indicar la ruta de la imagen que servirá como portadora.
* `--out` para indicar la ruta de la imagen de salida.
* `--steg` para indicar el algoritmo de esteganografía a utilizar.
* `--a` para indicar el algoritmo de cifrado simétrico a utilizar.
* `--m` para indicar el modo de operación del cifrado.
* `--pass` para indicar la contraseña del cifrado.

En el siguiente ejemplo se va a ocultar la imagen secreta `secret_img.bmp` en la imagen portadora `porter_img.bmp` con esteganografiado LSB1 y cifrado 3DES con modo de operación CBC y la imagen de salida que servirá como fachada para nuestros planes de destrucción será `facade_img.bmp`.

```sh {"id":"01JA3VH3QZQZQZQZQZQZQZQZQZ"}
./stegobmp --embed --in ./tests/secret_img.bmp  --p ./tests/porter_img.bmp  --out ./tests/facade_img.bmp --steg LSB1 --a 3des --m cbc --pass "secretpassword"
```

### Extraer información de una imagen BMP sin cifrado

Se debe indicar como primer argumento `--extract` para indicar que se va a extraer información de una imagen BMP. Los siguientes argumentos pueden variar su orden:
* `--p` para indicar la ruta de la imagen que se va a extraer la información.
* `--out` para indicar la ruta del archivo de salida.
* `--steg` para indicar el algoritmo de esteganografía a utilizar.

En el siguiente ejemplo se va a extraer la información oculta en la imagen `facade_img.bmp` con esteganografía LSB1 y se va a guardar en el archivo `secret_img.bmp`.

```sh {"id":"01JA3VH4QZQZQZQZQZQZQZQZQZ"}
./stegobmp --extract --p ./tests/facade_img.bmp --out ./tests/secret_img.bmp --steg LSB1
```

### Extraer información de una imagen BMP con cifrado

Se debe indicar como primer argumento `--extract` para indicar que se va a extraer información de una imagen BMP. Los siguientes argumentos pueden variar su orden:
* `--p` para indicar la ruta de la imagen que se va a extraer la información.
* `--out` para indicar la ruta del archivo de salida.
* `--steg` para indicar el algoritmo de esteganografía a utilizar.
* `--a` para indicar el algoritmo de cifrado simétrico a utilizar.
* `--m` para indicar el modo de operación del cifrado.
* `--pass` para indicar la contraseña del cifrado.

En el siguiente ejemplo se va a extraer la información oculta en la imagen `facade_img.bmp` con esteganografía LSB1 y cifrado 3DES con modo de operación CBC y se va a guardar en el archivo `secret_img.bmp`.

```sh {"id":"01JA3VH5QZQZQZQZQZQZQZQZQZ"}
./stegobmp --extract --p ./tests/facade_img.bmp --out ./tests/secret_img.bmp --steg LSB1 --a 3des --m cbc --pass "secretpassword"
```

## Operaciones soportadas

### Esteganografía
* LSB1
* LSB4
* LSBI

### Cifrado
* AES128
* AES192
* AES256
* 3DES

### Modos de operación
* ECB
* CBC
* CFB
* OFB
