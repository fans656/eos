#!/usr/bin/env python
import struct

from f6 import human_size

KERNEL_BEG = 0x8000
KERNEL_END = 0x80000

def get_section_headers(data):
    res = []
    for i in xrange(n_section_header_entries):
        base = section_header_offset + i * section_header_entry_size
        ph = data[base:base+section_header_entry_size]
        res.append(ph)
    return res

with open('bin/kernel.out', 'rb') as f:
    data = f.read()

section_header_offset = struct.unpack('<I', data[0x20:0x20+4])[0]
section_header_entry_size = struct.unpack('<H', data[0x2e:0x2e+2])[0]
n_section_header_entries = struct.unpack('<H', data[0x30:0x30+2])[0]

sections = []
kernel_beg = KERNEL_END
kernel_end = KERNEL_BEG
for i, ph in enumerate(get_section_headers(data)):
    beg = struct.unpack('<I', ph[0x0c:0x0c+4])[0]
    size = struct.unpack('<I', ph[0x14:0x14+4])[0]
    end = beg + size

    if KERNEL_BEG <= beg and beg < end and end <= KERNEL_END:
        image_offset = struct.unpack('<I', ph[0x10:0x10+4])[0]
        section_data = data[image_offset:image_offset + size]
        kernel_beg = min(kernel_beg, beg)
        kernel_end = max(kernel_end, end)
        sections.append((beg, section_data))

for i in xrange(len(sections)):
    sections[i] = (sections[i][0] - kernel_beg, sections[i][1])
sections.sort(key=lambda (i, _): i)

image = ['\x00'] * (kernel_end - kernel_beg)
for offset, section in sections:
    image[offset:offset+len(section)] = section
image = ''.join(image)

with open('bin/kernel.img', 'wb') as f:
    f.write(image)
