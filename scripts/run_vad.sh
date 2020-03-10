#!/bin/bash

# Be sure that this file has execution permissions:
# Use the nautilus explorer or chmod +x run_vad.sh


#DB=/home/albino/PAV/Prácticas/enunciados/obsoletos/P2/db.test
#DB=/mnt/c/Users/Patron/Desktop/2020\ Q2/PAV/LAB/P2/db.v4
DB=/home/patron/PAV/P2/db.v4
CMD=bin/vad  #write here the name and path of your program

for filewav in $DB/*/*wav; do
#    echo
    echo "**************** $filewav ****************"
    if [[ ! -f $filewav ]]; then 
	    echo "Wav file not found: $filewav" >&2
	    exit 1
    fi

    filevad=${filewav/.wav/.vad}

    $CMD -i $filewav -o $filevad || exit 1

# Alternatively, uncomment to create output wave files
#    filewavOut=${filewav/.wav/.vad.wav}
#    $CMD $filewav $filevad $filewavOut || exit 1

done

vad_evaluation.pl $DB/*/*lab

exit 0
