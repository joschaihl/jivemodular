# Why #
A fork of the amazing work done in the [Juced](http://code.google.com/p/juced) (aka Jucetice) and [Juce](http://rawmaterialsoftware.com/juce) projects.

Forked initially so the current amazing work in the Jost host (renamed "Jive" for this project) and various plugins can be documented and promoted.

# Downloads #
Download the zip on the left and copy the contained bundle to your **Applications** folder.

Here's a hastily knocked up [audio example](http://jivemodular.googlecode.com/files/JiveExample.mp3) of Jive in action.

# Features #
  * Modular plugin host supporting [VST](http://en.wikipedia.org/wiki/Virtual_Studio_Technology) and [ladspa](http://www.ladspa.org) plugins.
  * Basic analogue-style synthesiser VST plugin included (built from the wonderfully squelchy [xsynth dssi](http://dssi.sourceforge.net/) plugin).
  * Basic sampler VST plugin included (built from the powerful [discoDSP highlife](http://www.discodsp.com/highlife/)).
  * Rhythmic gater VST plugin included with control over attack, release and rate (including triplets).
  * Pattern sequencer - midi loops/patterns can be edited, saved and loaded to _sequencer_ plugins, which can be looped, or turned off.
  * Continuous controller automation for programming plugin parameter movement via MIDI sequences.
  * Supports arbitrarily complex MIDI and audio connections between plugins (i.e. sidechain compression, modulation etc is easily achieved).
  * Support for multiple external MIDI devices, and MIDI control of any plugin parameter.
  * Stem rendering - continuously records the audio out of specified plugins to WAV files for later editing in your favourite DAW (I recommend [Reaper](http://reaper.fm)).

# Status, Target Platforms #
Currently supported on Mac OS X 10.5 and newer (Leopard). Universal Binary supporting PowerPC and Intel Macs.

Thanks to the [Juce](http://rawmaterialsoftware.com/juce) library, no major barriers to running on Windows and Linux; some project maintenance needs to be done.

In general a work in progress, but active development and very usable at present.

# Screenshots #

Here are some images of Jive in action. If you want to hear what it sounded like, [download the audio example](http://jivemodular.googlecode.com/files/JiveExample.mp3).

![http://jivemodular.googlecode.com/files/JiveMainWindow.png](http://jivemodular.googlecode.com/files/JiveMainWindow.png)

Main window, with a drum sampler, two synths, and a few effects. Some [TAL](http://kunz.corrupt.ch) VST plugins are used alongside the Jive built in plugins.

![http://jivemodular.googlecode.com/files/LowlifeDrumkit.png](http://jivemodular.googlecode.com/files/LowlifeDrumkit.png)

The Lowlife sampler loaded up with kick, snare and hi hat samples - the drumkit in the [audio example](http://jivemodular.googlecode.com/files/JiveExample.mp3).

![http://jivemodular.googlecode.com/files/Xsynth.png](http://jivemodular.googlecode.com/files/Xsynth.png)

The Xsynth analoguish synth with the default patch. You can hear this as the whiny loop in the [audio example](http://jivemodular.googlecode.com/files/JiveExample.mp3).

![http://jivemodular.googlecode.com/files/CCAutomation.png](http://jivemodular.googlecode.com/files/CCAutomation.png)

A sequencer containing controller automation. In the [audio example](http://jivemodular.googlecode.com/files/JiveExample.mp3), you can hear this automating the cutoff frequency of a bandpass filter, provided by the colourful [BetaBugs Crayon Filter](http://www.betabugsaudio.com/plugs.php) VST plugin.

# Contact #
Problems or questions?

Email [jive@cartoonbeats.com](mailto:jive@cartoonbeats.com).