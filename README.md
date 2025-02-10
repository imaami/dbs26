```
      ▄▄  ▄▄                   ▄▄▄▄▄      ▄▄▄▄  
      ██  ██                  █▀▀▀▀██▄   ██▀▀▀█ 
 ▄███▄██  ██▄███▄   ▄▄█████▄        ██  ██ ▄▄▄  
██▀  ▀██  ██▀  ▀██  ██▄▄▄▄ ▀      ▄█▀   ███▀▀██▄
██    ██  ██    ██   ▀▀▀▀██▄    ▄█▀     ██    ██
▀██▄▄███  ███▄▄██▀  █▄▄▄▄▄██  ▄██▄▄▄▄▄  ▀██▄▄██▀
  ▀▀▀ ▀▀  ▀▀ ▀▀▀     ▀▀▀▀▀▀   ▀▀▀▀▀▀▀▀    ▀▀▀▀  
```

Generates all binary De Bruijn sequences with subsequence
length 6 (*all 67108864 of them*).

## Usage

```
  -h, --help            Help
  -b, --benchmark       Benchmark
  -o, --output <file>   Save to <file>
  -t, --threads <n>     Use <n> threads

When no arguments are given, computes the sequences using
all available logical CPUs and saves them to a file named
dbs26.bin in the current directory. Output data is always
raw binary uint64_t data in the native endianness.

Specifying the output file as a dash ('-') will print the
sequences to standard output in binary mode. Only do this
when redirecting the output to a file or another program.

On systems where xxd is available you can view the output
with the following (or similar) command:

  dbs26 -o- | xxd -e -g8 | less

Note: the size of the raw output is 512 MiB - be careful!
```

## Why only B(2,6)?

Because it's possible. Generating this specific set of sequences serves
as a proof of concept for the general approach.

The number of unique binary De Bruijn sequences with subsequence length
6 is a modest 67108864. Each sequence being 64 bits, i.e. 8 bytes, the
entire set takes up only 512 MiB. Going up just a single step to B(2,7)
explodes the full set to 144115188075855872. B(2,6) is the sweet spot
between "meh" and practically impossible.

## Does it scale?

Yes. The algorithm, while currently implemented and optimized for B(2,6)
exclusively, can be modified to generate longer De Bruijn sequences, and
I plan on doing so in the future.

## Why is the code ugly?

This project started as a quick single-file proof-of-concept on GitHub
Gist. At the time of writing, that state of affairs has not changed. My
primary goal after moving from Gist to a proper repository was to build
a cross-platform binary release for the mathematically inclined visitors
to explore. (Unfortunately, I'm not sure if there have been any so far.)
