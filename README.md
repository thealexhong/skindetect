# Simple Skin Detection Algorithm
Attempts to detect human limbs by looking at various skin colours for 640x480 images. Requires OpenCV libraries.

USAGE:

```
g++ skindetect.cpp -o skindetect `pkg-config --cflags --libs opencv`
./skindetect image.jpg
```
