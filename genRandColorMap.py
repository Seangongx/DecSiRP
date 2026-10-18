import random
from PIL import Image

random.seed(42)  # palette reproductible

img = Image.new("RGB", (256, 1))
for x in range(256):
    img.putpixel(
        (x, 0),
        (random.randrange(256),
         random.randrange(256),
         random.randrange(256))
    )

img.save("random.png")
