# Solución al capture the flag

> [!Note]
> Se asume que las ejecuciones se hacen sobre la raíz del proyecto (sino sólo agregar ../ a las rutas) 

El siguiente comando extrae un pdf en la carpeta [./stegoanalysis/out](../stegoanalysis/out).

```sh
stegobmp --extract -p ./assets/grupo9/avatar.bmp  --out ./stegoanalysis/out/avatar --steg LSBI

```

Si leemos el pdf vemos que dice:

> al .png cambiarle la extension por .zip y descomprimir

Con la siguiente extracción se obtiene el png en cuestión:

```sh
stegobmp --extract -p ./assets/grupo9/montevideo.bmp  --out ./stegoanalysis/out/montevideo --steg LSB1

```

Hacemos una copia para no alterar la imagen original que va a ser usada y cambiamos la extensión como nos dice el pdf:

```sh
cp ./stegoanalysis/out/montevideo.png ./stegoanalysis/out/montevideo_copy.zip

```

unzipeamos, 

```sh
unzip ./stegoanalysis/out/montevideo_copy.zip -d ./stegoanalysis/out/

```

1. Obtenemos la carpeta [sols](../stegoanalysis/out/sols) con un txt de instruccciones que indican cómo resolver el acertijo que como solución nos da la primitiva de encripción y el modo (AES ECB).
2. Miramos el [hexdump de paris](../stegoanalysis/hex/hexdumps_paris.hex) y econtramos en los últimos bytes la frase "la password es gloria". 
3. Usamos la información recolectada para ejecutar la última extracción obteniendo el "flag" que es un fragmento de una película.

```sh
stegobmp --extract -p ./assets/grupo9/anillo2.bmp  --out ./stegoanalysis/out/anillo2 --steg LSB4 -a aes256 -m ecb --pass gloria

```