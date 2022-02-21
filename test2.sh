#!/bin/bash



for ((i = 0;i<500;i++));
do
    let count=0;
    for f in $(ls ./tests/*.txt); do 
        ./a.out <$f > ./tests/`basename $f .txt`.output; 
    done;

    for f in $(ls ./tests/*.output); do
        diff -Bw $f  ./tests/`basename $f .output`.txt.expected > ./tests/`basename $f .output`.diff;
    done

    for f in $(ls tests/*.diff); do
        if [ -s $f ]; then
            cat $f
        else
            count=$((count+1));
        fi
    done

    if [ $count -ne 22 ]
    then
        echo $count;
    fi

    rm tests/*.output
    rm tests/*.diff
done

echo "finish"