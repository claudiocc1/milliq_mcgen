#! /bin/bash

#for FILE in `ls $1/m*/q*/*.cmd`; do
for FILE in `ls $1/m*/*.cmd`; do
    condor_submit $FILE
done

