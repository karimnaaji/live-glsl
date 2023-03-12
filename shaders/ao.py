# The MIT License (MIT)
# 
# Copyright (c) 2021 Karim Naaji
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in all
# copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
# SOFTWARE.
#
# Inspired from the papers "Computing Skylight for 2D Height Fields by Finding
# 1D Height Field Horizons in Amortized Constant Time" and "Scalable Height Field
# Self-Shadowing"
# 

from PIL import Image
import math

def vec3_sub(a, b):
    return [a[0] - b[0], a[1] - b[1], a[2] - b[2]]

def vec3_len(a):
    return math.sqrt(a[0] * a[0] + a[1] * a[1] + a[2] * a[2])

def occlusion(a, b):
    d = vec3_sub(b, a)
    length = vec3_len(d)
    return 0 if length == 0 else 1 - math.exp((-d[2] / length) * 0.5)

def slope(a, b):
    d = vec3_sub(a, b)
    length = vec3_len(d)
    z_diff = b[2] - a[2]
    return z_diff / length

def scanline_index(dx, dy, line, w, h):
    # left-bottom scan
    if dx > 0 and dy <= 0:
        if line < h: return line, 0      # left
        else: return h - 1, line - h + 1 # bottom
    # top-left scan
    elif dx >= 0 and dy > 0:
        if line < w: return 0, line      # top
        else: return line - w + 1, 0     # left
    # bottom-right scan
    elif dx <= 0 and dy < 0:
        if line < w: return h - 1, line  # bottom
        else: return line - w, w - 1     # right
    # right-top scan
    elif dx < 0 and dy >= 0:
        if line < h: return line, w - 1  # right
        else: return 0, line - h         # top

def scanline(im, px, px_ao, dx, dy, sample_count):
    scanline_count = abs(dx) * im.height + abs(dy) * im.width - abs(dy) * abs(dx)
    for scan in range(scanline_count):
        hull = []
        i, j = scanline_index(dx, dy, scan, im.width, im.height)
        hull.append([i, j, px[j, i][0]])
        while i >= 0 and i < im.height and j >= 0 and j < im.width:
            sample_z = px[j, i][0]
            while len(hull) > 1:
                slope0 = slope([i, j, sample_z], hull[-1])
                slope1 = slope([i, j, sample_z], hull[-2])
                if slope0 > slope1:
                    break
                hull.pop()
            horizon = hull[-1]
            hull.append([i, j, sample_z])
            prev_o = px_ao[j, i][0]
            o = occlusion([i, j, sample_z], horizon)
            o = int(prev_o + (1.0 / sample_count) * ((1 - o) * 255))
            px_ao[j, i] = (o, o, o, 255)
            i += dy
            j += dx

im = Image.open("heightmap_large.png")
px = im.load()
im_ao = Image.new("RGBA", [im.width, im.height])
px_ao = im_ao.load()

scanline(im, px, px_ao,  0,  1, 8)
scanline(im, px, px_ao,  0, -1, 8)
scanline(im, px, px_ao,  1,  0, 8)
scanline(im, px, px_ao, -1,  0, 8)
scanline(im, px, px_ao,  1,  1, 8)
scanline(im, px, px_ao, -1,  1, 8)
scanline(im, px, px_ao, -1, -1, 8)
scanline(im, px, px_ao,  1, -1, 8)

im_ao.save("heightmap_large_ao.png", "PNG")
im_ao.show()
