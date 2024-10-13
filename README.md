# cripto_steganography

# Compile the project

```sh {"id":"01JA3VGPSHQWK1WT026B2474MV"}
make all
```

# Usage example

First we will embed the secret image `secret_img.png` into the porter image `porter_img.bmp` and the output image that will serve as a facade for our destruction plans will be `facade_img.bmp`.

```sh {"id":"01JA3VH2E57SH82R7WBAG1AJ72"}
./stegobmp --embed --in ./tests/secret_img.png  -p ./tests/porter_img.bmp  --out ./tests/facade_img.bmp --steg LSB1 -a 3des -m cbc --pass "secretpassword"
```

After that we can see that the facade image looks disparate the the original counterpart but if we exectue the following we will se that the facade is hiding something. The output will be written into the `get_secret` file and the extension will be automatically added.

```sh {"id":"01JA3VHH590YV1Z54R845G7F4V"}
./stegobmp --extract -p ./tests/facade_img.bmp --out ./tests/get_secret --steg LSB1 -a 3des -m cbc --pass "secretpassword"
```