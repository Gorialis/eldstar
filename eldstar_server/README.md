Where `${START_FRAME}` is the first frame in the dump (`0` means the first image is `0.png`),
Where `${OUT}` is the name of the output,
Where `${FRAMERATE}` is the framerate (usually 60),

# Making transparent webm from frames
```sh
ffmpeg -f image2 -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "format=rgba,fps=${FRAMERATE}", -c:v libvpx -auto-alt-ref 0 -pix_fmt yuva420p ${OUT}.webm
```

# Making standard mp4 with mask mp4
```sh
ffmpeg -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "fps=${FRAMERATE}" -c:v libx264 -pix_fmt yuv420p ${OUT}.mp4
ffmpeg -r ${FRAMERATE} -start_number ${START_FRAME} -i %d.png -vf "colorchannelmixer=ra=1:rr=0:ga=1:gg=0:ba=1:bb=0,fps=${FRAMERATE}" -c:v libx264 -pix_fmt yuv420p ${OUT}_mask.mp4
```
