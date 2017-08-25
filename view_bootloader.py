import conf

with open(conf.IMAGE_FILE, 'rb') as f:
    s = f.read()
    print repr(s)
    print ['{:02x}'.format(ord(c)) for c in s[-2:]]
