#!/usr/bin/env bash
make clean
make all

# no_encryption folder
./stegobmp -extract -p ./assets/no_encryption/ladoLSB1.bmp -out ./tests/get_secret -steg LSB1
./stegobmp -extract -p ./assets/no_encryption/ladoLSB4.bmp -out ./tests/get_secret -steg LSB4
./stegobmp -extract -p ./assets/no_encryption/ladoLSBI.bmp -out ./tests/get_secret -steg LSBI

# encryption folder
./stegobmp -extract -p ./assets/encryption/ladoLSB1.bmp -out ./tests/get_secret -steg LSB1
./stegobmp -extract -p ./assets/encryption/ladoLSB4.bmp -out ./tests/get_secret -steg LSB4
./stegobmp -extract -p ./assets/encryption/ladoLSBI.bmp -out ./tests/get_secret -steg LSBI

./stegobmp -extract -p ./assets/encryption/ladoLSBIaes256ofb.bmp -out ./tests/get_secret -steg LSBI -a aes256 -m ofb -pass "margarita"
./stegobmp -extract -p ./assets/encryption/ladoLSBIdescfb.bmp -out ./tests/get_secret -steg LSBI -a 3des -m cfb -pass "margarita"
