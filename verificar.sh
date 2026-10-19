#!/bin/bash
# Uso: ./verificar.sh ./v0     (o ./v1, ./v2 ...)
# Compara el checksum de cada filtro con la tabla del enunciado.
B=${1:-./v0}
declare -A E=(
 [blur,2048]=15228573027849069044  [blur,4096]=14133797607195559027
 [gauss,2048]=6704049370287127316  [gauss,4096]=11810212636788029950
 [sobel,2048]=13749065433343460504 [sobel,4096]=15895088315003003173 )
for f in blur gauss sobel; do
  for n in 2048 4096; do
    got=$($B $f imgs/img$n.pgm | sed -n 's/.*checksum //p')
    [ "$got" == "${E[$f,$n]}" ] && s=OK || s="FALLA (obtuvo $got)"
    echo "$f $n: $s"
  done
done
echo "--- prueba.pgm (compare con README_curso.md) ---"
for f in blur gauss sobel; do echo "$f"; $B $f imgs/prueba.pgm | head -5; done
