#!/usr/bin/env bash
make clean
make all

# unencrypted
# ./stegobmp -embed -in ./tests/get_secret.png  -p ./tests/lado.bmp  -out ./tests/facade_img.bmp -steg LSBI
# ./stegobmp -extract -p ./tests/facade_img.bmp -out ./tests/secret -steg LSBI


./stegobmp -embed -in ./tests/get_secret.png  -p ./tests/porter_img.bmp  -out ./tests/facade_img.bmp -steg LSBI -a 3des -m cbc -pass "secretpassword"
./stegobmp -extract -p ./tests/facade_img.bmp -out ./tests/get_secret -steg LSBI -a 3des -m cbc -pass "secretpassword"


algorithms=("aes128" "aes192" "aes256" "3des")
modes=("ecb" "cfb" "ofb" "cbc")
# cbc y ecb no funcionan


# ./stegobmp -embed -in ./tests/output.png  -p ./tests/lado.bmp  -out ./tests/facade_img.bmp -steg LSB1 -a aes128 -m cbc -pass "secretpassword"
# ./stegobmp -extract -p ./tests/facade_img.bmp -out ./tests/get_secret -steg LSB1 -a aes128 -m cbc -pass "secretpassword"