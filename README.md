# Name
**datmap2png** - render TacOps .dat map into a PNG image

# Synopsis
**datmap2png** [-dhv] [--1km] [--5km] [--10km] -i FILENAME -o FILENAME

# Description
The program reads TacOps .dat map file and produces a nice render of the terrain
and all of the features into a PNG file. The rendering style and the color scheme
are similar to one found in the original TacOps simulation program (available 
under the menu "Terrain analysis").

The options are as follows:

```
Help Options:
  -h, --help               Show help options

Application Options:
  -i, --input=FILENAME     TacOps .dat map filename
  -o, --out=FILENAME       Output image filename
  -v, --verbose            Be verbose
  --1km                    Overlay 1-kilometer grid
  --5km                    Overlay 5-kilometer grid
  --10km                   Overlay 10-kilometer grid
  -d, --description        Add description text to generated image
```

# Examples
The invocation

```
datmap2png -vd --1km -i Map001c.dat -o Map001c.png
```

Will produce a render of the Map001c and append text description and 1km grid to
the resulting output image.

# Compatibility
The data format used in TacOps maps is proprietary/undocumented and has been reverse-engineered
by the author of this software. All "Type 3" maps found in most recent 
versions of TacOps are thought to be supported.

# Bugs
The binary structure is mostly known (guessed), but parsing surprises are 
not impossible. 
