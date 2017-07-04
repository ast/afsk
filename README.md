# AFSK AX.25 Erlang port

This is an Erlang [port](http://erlang.org/doc/reference_manual/ports.html) program written in plain C for decoding AX.25 frames sent using 1200 baud AFSK (Audio Frequency Shift Keying). This is most frequently (no pun intended) used today for APRS (Automatic Packet Reporting System).

The decoded frames are written raw to `stdout` in the `{packet,2}` format.

The idea here is to take advantage of Erlangs excellent facilities for parsing binary data into packets.

## Getting Started

I have not yet incorporated the code for reading real time samples from audio hardware. I'm testing the program using recorded APRS traffic recorded by WA8LMF. More info below.

### Prerequisites

Right now, to build this early version of the project, you need `Xcode` and `libsndfile`. I'm planning a Linux version.

## Testing

I have been testing this program with the *excellent* test data provided by **WA8LMF Stephen Smith**.

Stephen has created a TNC test CD that you can download [here](http://wa8lmf.net/TNCtest/).

The downloaded `.zip` file contains a `.bin` and `.cue` file for burning to CD. You can convert it to wav files that are more practical these days, using the excellent `bchunk` found [here](http://he.fi/bchunk/).

Convert to wave files:

`$ bchunk -w TNC_Test_Ver-1.1.bin TNC_Test_Ver-1.1.cue TNC_Test`

Remove one channel to make it mono, downsample to 24k/s and trim the first 4.75s of speech / silence.

`$ sox TNC_Test01.wav TNC_Test01-mono242.wav remix 1 rate 24k trim 4.75`

Then give the path to one of these files as the first argument of the program. The program will try to decode any afsk ax.25 frames.

## Built With

* AFSK AX.25 code heavily inspired by [multimon-ng](https://github.com/EliasOenal/multimon-ng)
* [libsndfile](http://www.mega-nerd.com/libsndfile/)
* CRC code from [WAMPES](https://github.com/dieterdeyke/WAMPES) by Dieter Deyke

## Contributing

I'd be happy to receive ideas and suggetions!

## Authors

* **Albin Stigö** - *Initial work* - [albinstigo.me](http://albinstigo.me)

## License

This project is licensed under the MIT License - see the [LICENSE.md](LICENSE.md) file for details

## Acknowledgments

* The ring buffer code was inspired by [Juho Snellman](https://www.snellman.net/blog/archive/2016-12-13-ring-buffers/)
* The DSP code was inspired by [The Scientist and Engineer's Guide to Digital Signal Processing](http://www.dspguide.com), an amazing book!
