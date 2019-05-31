rm binmeu.txt
rm bincerto.txt
hexdump -Cv binario-1.index > bincerto.txt
hexdump -Cv binmeu.bin > binmeu.txt
meld bincerto.txt binmeu.txt
