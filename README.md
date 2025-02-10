```
      ▄▄  ▄▄                   ▄▄▄▄▄      ▄▄▄▄  
      ██  ██                  █▀▀▀▀██▄   ██▀▀▀█ 
 ▄███▄██  ██▄███▄   ▄▄█████▄        ██  ██ ▄▄▄  
██▀  ▀██  ██▀  ▀██  ██▄▄▄▄ ▀      ▄█▀   ███▀▀██▄
██    ██  ██    ██   ▀▀▀▀██▄    ▄█▀     ██    ██
▀██▄▄███  ███▄▄██▀  █▄▄▄▄▄██  ▄██▄▄▄▄▄  ▀██▄▄██▀
  ▀▀▀ ▀▀  ▀▀ ▀▀▀     ▀▀▀▀▀▀   ▀▀▀▀▀▀▀▀    ▀▀▀▀  
```

Generates all binary De Bruijn sequences with subsequence length 6
(*all 67108864 of them*).

## Why only B(2,6)?

Because it's possible. Generating *and storing* every unique B(2,6) can
be done in a reasonable amount of time and space, making it a good proof
of concept for the general approach.

The number of unique binary De Bruijn sequences with subsequence length
6 is a modest 67108864. Each sequence being 64 bits, i.e. 8 bytes, the
entire set takes up only 512 MiB. Going up just a single step to B(2,7)
explodes the sequence count to 144115188075855872, while B(2,5) is just
2048 sequences. B(2,6) is the sweet spot between trivial and impossible.

## Does it scale? Can you do B(2,7) or larger?

Yes. The algorithm, while currently implemented and optimized for B(2,6)
exclusively, can be modified to generate longer De Bruijn sequences, and
I plan on doing so in the future. None of the larger sets of binary De
Bruijn sequences can be generated in their entirety, but this algorithm
can be extended to generate sub-ranges of them starting at any arbitrary
prefix, as long as the prefix contains only unique subsequences.

## Why is the code ugly?

This project started as a quick single-file proof-of-concept on GitHub
Gist. At the time of writing, that state of affairs has not changed. My
primary goal after moving from Gist to a proper repository was to build
a cross-platform binary release for the mathematically inclined visitors
to explore. (Unfortunately, I'm not sure there have been any.)

## Usage

This is a command-line program, so regardless of your OS of choice you
will need to open up a terminal. The details can be found by running
`dbs26 --help`, which will print the following:

```
Usage: dbs26 [-o <file>] [-t <n>]
       dbs26 -b [-t <n>]
       dbs26 -h

Generates all binary De Bruijn sequences with subsequence
length 6 (all 67108864 of them).

Options:
  -h, --help            Print this help message and exit
  -b, --benchmark       Only benchmark, don't output data
  -o, --output <file>   Save output to <file> (dbs26.bin)
  -t, --threads <n>     Use <n> threads (available cores)

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

## Footnotes: things I'm still working on

- The program could use more features; suggestions are welcome.
- There's no description of the algorithm. I realize this is a major
  problem for non-programmers. Will fix as schedule allows.
- The algorithm would serve well as a library, but its usefulness is
  currently very limited from that perspective.
- The algorithm is currently recursive. Will fix when adding support
  for longer De Bruijn sequences.
- There's no makefile, and the code is a single file. Will add build
  scripts and a proper project structure as I go.
