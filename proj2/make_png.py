# pillow
from PIL import Image

img1 = Image.new("RGB", (20, 20), color="white")
img1.save("assets/ball.png")

img2 = Image.new("RGB", (20, 100), color="white")
img2.save("assets/paddle.png")