from PIL import Image

width, height = 256, 256
image = Image.new("RGBA", (width, height), (255, 255, 255, 255))
pixels = image.load()

for y in range(height):
    for x in range(width):
        if (x // 32 + y // 32) % 2 == 0:
            pixels[x, y] = (200, 50, 50, 255) # Red-ish
        else:
            pixels[x, y] = (50, 50, 200, 255) # Blue-ish

image.save("checkerboard.png")
