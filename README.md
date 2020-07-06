# Audio Transport

This code implements a novel effect for transitioning between audio signals that we call "audio transport". As an interpolation parameter is changed, the pitches in one signal slide to the pitches in the other, producing a portamento, or musical glide. The assignment of pitches in one sound to pitches in the other is accomplished by solving a 1-dimensional optimal transport problem.

The effect is described in [this paper](https://arxiv.org/abs/1906.06763). Audio examples can be found [here](https://soundcloud.com/audio_transport), or click on the image below for a video demonstration:

[![video](https://live.staticflickr.com/65535/49050087898_a81680c7cb_o_d.png)](https://www.youtube.com/watch?v=LXqZMKXSPJo)

This code provides the *static* effect, meaning it provides an executable that takes two audio files as input and combines the two using the effect to produce and output audio file. The code demonstrated in the video that uses the effect *live* uses the functions provided by this library on [portaudio](http://www.portaudio.com/) streams. The live code is super hacky and you could probably make something much better, which is why I haven't included it here. But if you really want it, I can send it to you. All of the important stuff is in this library.

## Usage

### Static Use

Using the static effect does not require writing any more code but it does require several dependencies. You will need [```fftw3```](http://fftw.org/), [```ffmpeg```](https://ffmpeg.org/) and [```audiorw```](https://github.com/sportdeath/audiorw).

Install ```fftw3``` and ```ffmpeg``` via your system package manager. This code has been tested on versions 3.3.8 and 4.2.3 respectively on a machine running Arch Linux. Hopefully things don't break on your system but if they do, please make a pull request :-) !

 Then install the ```audiorw``` library with ```cmake```:

    git clone https://github.com/sportdeath/audiorw
    mkdir audiorw/build
    cd audiorw/build
    cmake ..
    make
    sudo make install

Once the dependencies are installed, install ```audio_transport``` with ```cmake```:

    git clone https://github.com/sportdeath/audio_transport
    mkdir audio_transport/build
    cd audio_transport/build
    cmake .. -D BUILD_EXAMPLES=ON
    make

Then, to apply the effect use the ```transport``` binary. For example, below we tranform piano audio into guitar audio. The output starts by sounding like the piano audio for the first 20% of the duration. Then between 20% and 70% of the duration, the piano is transformed into a guitar. In the last 30% the audio is simply the guitar.

    ./transport piano.wav guitar.mp3 20 70 out.flac

### External Use

If you want to use the audio transport functions provided by this library in another project (*e.g.* to make a live effect) then this library only requires [```fftw3```](http://fftw.org/). Once you have it, install ```audio_transport``` with ```cmake```:

    git clone https://github.com/sportdeath/audio_transport
    mkdir audio_transport/build
    cd audio_transport/build
    cmake ..
    sudo make install

Then in your own project, include these headers:

    #include <audio_transport/spectral.hpp>
    #include <audio_transport/audio_transport.hpp>

```spectral.hpp``` provides functions that turn vectors of audio into spectral objects, incapsulating time and frequency as well as their [reassigned counterparts](https://en.wikipedia.org/wiki/Reassignment_method) which are necessary for the effect. It also provides the inverse.

```audio_tranport.hpp``` provides an ```interpolate``` function that takes windows of audio (that are in the ```spectral``` format) and combines them according the effect.
