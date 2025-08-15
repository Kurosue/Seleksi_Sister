import subprocess
from pwn import *

input_so_far = ["a"]
idx = 0
flag = ''
while True:
    c = 'c\n' * idx
    with open("gdbscript.txt", "w") as f:
        f.write(
    f"""set pagination off
    set confirm off
    break *0x000000000040538d
    run <<< "{''.join(input_so_far)}"
    {c}
    x/s $rax
    quit
    """)

    result = subprocess.run(
        ['gdb', '--batch', '-x', 'gdbscript.txt', './dots'],
        stdout=subprocess.PIPE,
        stderr=subprocess.PIPE
    )

    output = result.stdout.decode()
    lines = output.strip().splitlines()
    output = lines[-1].split()[-1][1]
    info(f"Output : {output}")

    if output == '}':
        log.success(f"Found the flag: {''.join(input_so_far[:-1]) + output}")
        flag = ''.join(input_so_far[:-1]) + output
        break

    input_so_far[idx] = output
    input_so_far.append('a')
    idx += 1
info('Testing the final input...')
io = process('./dots')
io.sendline(flag.encode())
res = io.recvall(timeout=5)
log.success(f"Result : \n{res.decode()}")

