#!/usr/bin/env bash
make clean
make all

algorithms=("aes128" "aes192" "aes256" "3des")
modes=("ecb" "cfb" "ofb" "cbc")
steg_methods=("LSB1" "LSB4" "LSBI")

secret_img="./tests/secret_img.png"
porter_img="./tests/porter_img.bmp"
facade_img="./tests/facade_img.bmp"
get_secret="./tests/get_secret"
password="secretpassword"

# Unencrypted
for steg in "${steg_methods[@]}"; do
    echo "Embedding with $steg (unencrypted)"
    ./stegobmp --embed --in "$secret_img" -p "$porter_img" --out "$facade_img" --steg "$steg"
    echo "Extracting with $steg (unencrypted)"
    ./stegobmp --extract -p "$facade_img" --out "$get_secret" --steg "$steg"
done

# Encrypted
for alg in "${algorithms[@]}"; do
    for mode in "${modes[@]}"; do
        for steg in "${steg_methods[@]}"; do
            echo "Embedding with $steg, algorithm $alg, mode $mode"
            ./stegobmp --embed --in "$secret_img" -p "$porter_img" --out "$facade_img" --steg "$steg" -a "$alg" -m "$mode" --pass "$password"
            echo "Extracting with $steg, algorithm $alg, mode $mode"
            ./stegobmp --extract -p "$facade_img" --out "$get_secret" --steg "$steg" -a "$alg" -m "$mode" --pass "$password"
        done
    done
done