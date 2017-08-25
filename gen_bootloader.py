import conf

with open(conf.IMAGE_FILE, 'wb') as f:
    f.write('\0' * 510 + '\x55\xaa')
