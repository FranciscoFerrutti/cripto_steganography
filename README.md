# cripto_steganography

stegobmp --embed --in ./tests/secret.txt  -p ./tests/lado.bmp  --out ./tests/out.bmp --steg LSB1 -a 3des -m cbc --pass "secretpassword"

stegobmp --extract -p ./tests/out.bmp --out ./tests/desecret.txt --steg LSB1 -a 3des -m cbc --pass "secretpassword"