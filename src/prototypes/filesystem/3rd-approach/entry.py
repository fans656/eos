from conf import *


class Entry(object):

    @property
    def fpath(self):
        names = []
        cur = self
        while cur:
            names.append(cur.name)
            cur = cur.parent
        s = '/'.join(reversed(names))
        if not s.startswith('/'):
            s = '/' + s
        return s

    @property
    def is_dir(self):
        return self.type == TYPE_DIRECTORY

    @property
    def parent(self):
        return self.get_entry_by_i_block(self.i_parent)

    @property
    def prev_sibling(self):
        return self.get_entry_by_i_block(self.i_prev_sibling)

    @property
    def next_sibling(self):
        return self.get_entry_by_i_block(self.i_next_sibling)
