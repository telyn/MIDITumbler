MidiTumbler
========

This is the git repository, for the hardcore hacker types. If you just want to ***use*** MidiTumbler, download it from [my website](http://aetheria.co.uk/miditumbler.html).

It's been built for Windows and OS X succesfully. A Linux port is unlikely.

Setting up a build environment
====================

You'll need a copy of Box2D compiled as a static library, as well as the VST SDK.
I set mine up so that in my Visual Studio 2010 folder I have a lib folder, into which I have put the Box2D source and VST SDK, as Box2D_v2.2.1 and vstsdk2.4, respectively. The project file uses relative paths, so you can do the same without having to touch the project properties. Obviously it's easily customisable :-)

