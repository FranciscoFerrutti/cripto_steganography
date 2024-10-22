Esto debería generar un pdf

```sh
stegobmp --extract -p ./assets/grupo9/avatar.bmp  --out ./tests/output --steg LSBI
```

el pdf dice

> al .png cambiarle la extension por .zip y descomprimir

```sh
stegobmp --extract -p ./assets/grupo9/montevideo.bmp  --out ./tests/buscaminas_png --steg LSB1
```

cambiamos el .png a .zip