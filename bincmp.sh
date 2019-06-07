rm binmeu.txt
rm bincerto.txt
hexdump -Cv binario-7-depois.index > bincerto.txt
hexdump -Cv binario-7.index > binmeu.txt
meld bincerto.txt binmeu.txt
