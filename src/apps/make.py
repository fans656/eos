#!/usr/bin/env python
# https://en.wikipedia.org/wiki/Executable_and_Linkable_Format
import struct
import os
import math

KERNEL_BEG = 0xf00000
KERNEL_END = KERNEL_BEG + 1024 * 1024

class ELF(object):

    def __init__(self, data):
        self.magic = unpack(data, 0x00, 4)
        self.bits = unpack(data, 0x04, 1)
        self.entry = unpack(data, 0x18, 4)

        segments_offset = unpack(data, 0x1c, 4)
        n_segments = unpack(data, 0x2c, 2)
        segment_entry_size = unpack(data, 0x2a, 2)
        base = segments_offset
        size = n_segments * segment_entry_size
        self.segments = get_entries(
            data[base:base+size], n_segments, segment_entry_size, Segment)

        idx_names_section = unpack(data, 0x32, 2)

        sections_offset = unpack(data, 0x20, 4)
        n_sections = unpack(data, 0x30, 2)
        section_entry_size = unpack(data, 0x2e, 2)
        base = sections_offset
        size = n_sections * section_entry_size
        self.sections = get_sections(
            data, idx_names_section,
            data[base:base+size], n_sections, section_entry_size)

    def show(self):
        print '=' * 20, 'ELF file'
        print 'Entry: {:08x}'.format(self.entry)
        print '-' * 20, 'Segments'
        for entry in self.segments:
            entry.show()
        print '-' * 20, 'Sections'
        for entry in self.sections:
            entry.show()
        print '=' * 20, 'End of ELF file'
        print

def get_entries(data, n_entries, entry_size, constructor):
    entries = []
    for i in xrange(n_entries):
        base = i * entry_size
        entries.append(constructor(data[base:base+entry_size]))
    return entries

def get_sections(g_data, idx_names_section, data, n_entries, entry_size):
    sections = get_entries(data, n_entries, entry_size,
                           lambda data: Section(g_data, data))

    names_section = sections[idx_names_section]
    base = names_section.image_offset
    size = names_section.size
    names_content = g_data[base:base+size]

    for section in sections:
        section.name = names_content[section.name_offset:]
        section.name = section.name[:section.name.index('\x00')]

    sections.sort(key=lambda s: s.addr + s.size)

    return sections

class Segment(object):

    def __init__(self, data):
        self.data = data
        self.vaddr = unpack(data, 0x08, 4)
        self.memsz = unpack(data, 0x14, 4)

    def show(self):
        print '{:08x} {:08x}'.format(
            self.vaddr, self.vaddr + self.memsz)

class Section(object):

    def __init__(self, g_data, data):
        self.addr = unpack(data, 0x0c, 4)
        self.size = unpack(data, 0x14, 4)
        self.name_offset = unpack(data, 0x00, 4)
        self.image_offset = unpack(data, 0x10, 4)

        self.beg = self.addr
        self.end = self.beg + self.size

        self.data = g_data[self.image_offset:self.image_offset+self.size]

    def show(self):
        print '{name:20} {beg:08x} {end:08x} {size}'.format(
            name=self.name,
            beg=self.addr,
            end=self.addr + self.size,
            size=self.size,
        )

def unpack(data, offset, size):
    if size == 1:
        fmt = '<B'
    elif size == 2:
        fmt = '<H'
    elif size == 4:
        fmt = '<I'
    else:
        raise Exception('unsupported size')
    return struct.unpack(fmt, data[offset:offset+size])[0]

def print_hex(s):
    print ' '.join('{:02x}'.format(ord(t)) for t in s)

# read kernel.out
with open('a.out', 'rb') as f:
    data = f.read()
elf = ELF(data)

# parse ELF sections
sections = elf.sections
kernel_beg = KERNEL_END
kernel_end = KERNEL_BEG
valid_sections = []
for section in sections:
    beg = section.beg
    end = section.end
    size = section.size
    if KERNEL_BEG <= beg and beg < end and end <= KERNEL_END:
        image_offset = section.image_offset
        section_data = data[image_offset:image_offset + size]
        kernel_beg = min(kernel_beg, beg)
        kernel_end = max(kernel_end, end)
        valid_sections.append(section)
sections = valid_sections

# generate kernel image
image = ['\x00'] * (kernel_end - kernel_beg)
for section in sections:
    offset = section.beg - elf.entry
    if section.name == '.bss':
        section.data = '\x00' * section.size
    image[offset:offset+section.size] = section.data
image = ''.join(image)
size = len(image)

with open('a.img', 'wb') as f:
    f.write(image)
