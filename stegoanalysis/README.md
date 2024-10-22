Esto debería generar un pdf

```sh
stegobmp --extract -p ./assets/grupo9/avatar.bmp  --out ./stegoanalysis/out/avatar --steg LSBI
```

el pdf dice

> al .png cambiarle la extension por .zip y descomprimir

```sh
stegobmp --extract -p ./assets/grupo9/montevideo.bmp  --out ./stegoanalysis/out/montevideo.png --steg LSB1
```

Hacemos una copia

```sh
cp ./stegoanalysis/out/montevideo.png ./stegoanalysis/out/montevideo_copy.png
```

cambiamos el .png a .zip

```sh
unzip ./stegoanalysis/out/montevideo_copy.zip -d ./stegoanalysis/out/
```

1. Aparece la carpeta sols con un txt de instruccciones que nos diecen usar AES ECB
2. Miramos el hexdump de paris y econtramos password "gloria"

```sh
stegobmp --extract -p ./assets/grupo9/anillo2.bmp  --out ./stegoanalysis/out/anillo2.wmb --steg LSB4 -a aes256 -m ecb --pass gloria
```