#!/usr/bin/env bash

# Function to install fish if it's not already installed
install_fish() {
    if ! command -v fish &> /dev/null; then
        echo "Fish shell not found."
        read -p "Do you want to install Fish shell? (y/n): " confirm
        if [[ "$confirm" =~ ^[Yy]$ ]]; then
            echo "Installing Fish..."
            sudo apt update
            sudo apt install -y fish
        else
            echo "Fish shell is required to run this script. Exiting."
            exit 1
        fi
    else
        echo "Fish shell is already installed."
    fi
}

# Install Fish shell if necessary
install_fish

# Execute the script in Fish shell
fish -c '
    make clean
    make all

    set algorithms aes128 aes192 aes256 3des
    set modes ecb cfb ofb cbc
    set steg_methods LSB1 LSB4 LSBI

    set secret_img ./tests/secret_img.png
    set porter_img ./tests/porter_img.bmp
    set facade_img ./tests/facade_img.bmp
    set get_secret ./tests/get_secret
    set password secretpassword

    # Unencrypted
    for steg in $steg_methods
        echo "Embedding with $steg (unencrypted)"
        ./stegobmp -embed -in $secret_img -p $porter_img -out $facade_img -steg $steg
        echo "Extracting with $steg (unencrypted)"
        ./stegobmp -extract -p $facade_img -out $get_secret -steg $steg
    end

    # Encrypted
    for alg in $algorithms
        for mode in $modes
            for steg in $steg_methods
                echo "Embedding with $steg, algorithm $alg, mode $mode"
                ./stegobmp -embed -in $secret_img -p $porter_img -out $facade_img -steg $steg -a $alg -m $mode -pass $password
                echo "Extracting with $steg, algorithm $alg, mode $mode"
                ./stegobmp -extract -p $facade_img -out $get_secret -steg $steg -a $alg -m $mode -pass $password
            end
        end
    end
'
