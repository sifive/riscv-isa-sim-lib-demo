
# To build:

## If Spike is installed on the system:
run `make`

## If using a custom Spike build:

SPIKE_INCLUDE should be set to the directory containing the Spike headers
SPIKE_LIBDIR should be set to the directory containing the Spike libraries

e.g.
```
make SPIKE_INCLUDES=-I/scratch/aryoung/riscv-isa-sim/ SPIKE_LIBDIR=-L/scratch/aryoung/riscv-isa-sim/build
```

# To Run:

## If Spike is installed on the system:
run `./demo`

## If using a custom Spike build:

Set the LD_LIBRARY_PATH to the directory containing the Spike libraries.  This should be the same
as the value of SPIKE_LIBDIR used during the build.  For example:
```
LD_LIBRARY_PATH=$SPIKE_LIBDIR ./demo
```

or copy the libraries to the same directory as the executable first:
```
cp $SPIKE_LIBDIR/*.so .
./demo
```

or symlink the libraries to the same directory as the executable:
```
ln -s $SPIKE_LIBDIR/ ./lib
./demo
```
