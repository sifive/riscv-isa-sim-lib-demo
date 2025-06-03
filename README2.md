# To build:

First, `cd src`.

## If Spike is installed on the system:

run `make`

## If using a custom Spike build:

`CFLAGS` should be set to include the directory containing the Spike headers.
`LDFLAGS` should be set to include the directory containing the Spike libraries.

For example,
```
make CFLAGS=-I../riscv-isa-sim/ LDFLAGS=-L../riscv-isa-sim/build
```

# To Run:

## If Spike is installed on the system:

run `./demo`

## If using a custom Spike build:

Set the LD_LIBRARY_PATH to the directory containing the Spike libraries.  This should be the same
as the value used for `LDFLAGS` used during the build.  For example:
```
LD_LIBRARY_PATH=../riscv-isa-sim/build ./demo
```

or copy the libraries to the same directory as the executable first:
```
cp ../riscv-isa-sim/build/*.so .
./demo
```

or symlink the library directory to the lib directory beside the executable:
```
ln -s $SPIKE_LIBDIR/ ./lib
./demo
```
