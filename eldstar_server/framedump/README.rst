
Note
=====

This document is formatted in reStructuredText.
If you do not have any readers or programs that can read reStructuredText,
consider reading from the auto-formatted version on GitHub:

https://github.com/pm-reverse-engineering/eldstar/blob/master/eldstar_server/framedump/README.rst


Summary
========

The ``framedump`` folder in the Eldstar distribution is the folder in which the Eldstar server will dump frames when in recording mode.

A frame is dumped (without UI) whenever the server receives an update from a connected client.
It is formatted as an RGBA PNG file, as ``<frame number>.png``.

This can be used to create an Eldstar capture of, say, a TAS recording, by turning Eldstar to recording mode and replaying the TAS with a client script enabled.

You can reformat these PNGs into a video using software of your choice, or using ``ffmpeg`` on the command line as per the instructions below.


Encoding
=========

You can encode the framedump sequence into many different formats depending on your use case.

If you plan to use the video for editing software such as After Effects, Premiere Pro or Sony Vegas,
or you are uploading this encoding directly to video sharing services like YouTube, I recommend encoding to MP4 [libx264+yuv420p].

For MP4, you can also encode an additional transparency mask that allows you to place it against another background in editing software (see https://youtu.be/BBzppoW9DMw).

Encoding to a WEBM with direct transparency is possible but many video players and editing softwares will not support it.

Encoding commands
-----------------
Where:

- ``${START_FRAME}`` is the first frame in the framedump (``0`` means ``0.png`` is the first frame present)
- ``${OUT}`` is the name of the output file, such as ``tas_output``
- ``${FRAMERATE}`` is the framerate of the dump (When encoding with BizHawk on US/JP (NTSC), this will be 60, but on EU (PAL) it will be 50)

Encoding frame data w/ alpha as WEBM (libvpx+yuva420p)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code:: bash

    ffmpeg -f image2 -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "format=rgba,fps=${FRAMERATE}", -c:v libvpx -auto-alt-ref 0 -pix_fmt yuva420p ${OUT}.webm

Encoding frame data as MP4 (libx264+yuv420p)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code:: bash

    ffmpeg -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "fps=${FRAMERATE}" -c:v libx264 -pix_fmt yuv420p ${OUT}.mp4

Encoding mask video as MP4 (libx264+yuv420p)
~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
.. code:: bash

    ffmpeg -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "colorchannelmixer=ra=1:rr=0:ga=1:gg=0:ba=1:bb=0,fps=${FRAMERATE}" -c:v libx264 -pix_fmt yuv420p ${OUT}_mask.mp4
