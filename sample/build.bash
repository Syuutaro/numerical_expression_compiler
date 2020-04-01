#compile src file
../app sample.txt

#assemble
gcc -c sample.s

#link and generate execution file
gcc sample.o -o sample

#execute
./sample
