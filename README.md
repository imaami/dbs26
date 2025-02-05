# dbs26

Generates all binary De Bruijn sequences with subsequence
length 6 (*all 67108864 of them*).

## Usage

<pre>
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
</pre>
