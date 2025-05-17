# Xpander

A eurorack module to compress audio signals. It is based on the Compressor module by MKI.

This compressor has dual channels, each with its own set of controls. The controls are as follows:

- **Threshold**: Sets the level at which the compressor starts to work. Signals above this level will be compressed.
- **Ratio**: Sets the amount of compression applied to the signal. A higher ratio means more compression.
- **Attack**: Sets the time it takes for the compressor to start working after the signal exceeds the threshold. A
  shorter attack time means the compressor will respond more quickly to peaks in the signal.
- **Release**: Sets the time it takes for the compressor to stop working after the signal falls below the threshold. A
  shorter release time means the compressor will stop working more quickly.
- **Makeup Gain**: Adds gain to the compressed signal to bring it back up to a desired level.
- **Gain**: Controls the overall input level of the channels.

Yes, the name is a reference to the song Xpander By Sasha, for some reason I found it a fitting name.

## Usage

The Xpander module is designed to be used in a eurorack modular synthesizer setup. It can be used to compress audio
signals from various sources, such as synthesizers, drum machines, or other audio devices. The module can be used to
control the dynamic range of the audio signal, making it more consistent and polished.
