KB = 1024
MB = 1024 * KB
GB = 1024 * MB

size = 16 * MB

IMAGE_FPATH = '../bin/eos.img'

with open(IMAGE_FPATH, 'rb+') as f:
    f.seek(size - 1)
    f.write('\0')
