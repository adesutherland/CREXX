# I/O, Sockets, Processes, And Time

File I/O, binary I/O, sockets, process spawning, and clock/time operations.

Generated skeleton from `cmake-build-debug/bin/rxas -i`; edit prose by hand and regenerate placeholder inventories only with care.

Instruction placeholders in this section: 44.

## Section Checklist

- Purpose overview: TODO
- Operand conventions: TODO
- Signal/error behavior: TODO
- Examples: TODO
- Cross-links to related instructions: TODO

## `fclearerr`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e3` | `{REG}` | clearerr op1 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fclose`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d9` | `{REG,REG}` | op1 rc(int) = fclose op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `feof`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e4` | `{REG,REG}` | op1 rc(int) = feof op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `ferror`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e5` | `{REG,REG}` | op1 rc(int) = ferror op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fflush`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01da` | `{REG,REG}` | op1 rc(int) = fflush op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fopen`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d8` | `{REG,REG,REG}` | op1 file*(int) = fopen filename op2(string) mode op3(string) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `freadb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01db` | `{REG,REG,REG}` | op1(binary) = fread op2 file*(int) op3 bytes(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `freadbyte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01dd` | `{REG,REG}` | op1 (int) = read byte op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `freadcdpt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01de` | `{REG,REG}` | op1 (string and int) = read codepoint op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `freadline`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01dc` | `{REG,REG}` | op1 (string) = read until newline op2 file*(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fwrite`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01df` | `{REG,REG}` | fwrite to op1 file*(int) from op2(string) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fwriteb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e0` | `{REG,REG}` | fwrite to op1 file*(int) from op2(binary) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fwritebyte`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e1` | `{REG,REG}` | write byte to op1 file*(int) op2 source(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `fwritecdpt`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01e2` | `{REG,REG}` | write codepoint to op1 file*(int) op2 source(int) |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `getenv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ce` | `{REG,REG}` | get environment variable, op1=env[op2] |
| `0x01cf` | `{REG,STRING}` | get environment variable, op1=env[op2] |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `mtime`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01cb` | `{REG}` | Put time in microseconds into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `readline`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01c9` | `{REG}` | Read Line to op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `say`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01c2` | `{REG}` | Say op1 |
| `0x01c5` | `{INT}` | Say op1 |
| `0x01c6` | `{FLOAT}` | Say op1 |
| `0x01c7` | `{STRING}` | Say op1 |
| `0x01c8` | `{CHAR}` | Say op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sayx`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01c3` | `{REG}` | Say op1 without line feed |
| `0x01c4` | `{STRING}` | Say op1 (as string) without line feed |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockaccept`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0043` | `{REG,REG}` | Accept connection from op2 server handle, client handle in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockbind`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0041` | `{REG,REG,REG}` | Bind op1 socket handle to host op2 and port op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockblocking`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004b` | `{REG,REG,REG}` | Set op2 socket blocking flag op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockclose`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003f` | `{REG,REG}` | Close op2 socket handle, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockconnect`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0040` | `{REG,REG,REG}` | Connect op1 socket handle to host op2 and port op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockconnecttls`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0053` | `{REG,REG,REG}` | Connect op1 socket handle to host op2 and port op3 with client TLS |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockerror`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0051` | `{REG,REG}` | Read last socket error text for op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockkeepalive`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004d` | `{REG,REG,REG}` | Set op2 socket SO_KEEPALIVE flag op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socklisten`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0042` | `{REG,REG,REG}` | Listen on op2 socket handle with backlog op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socklocal`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004f` | `{REG,REG}` | Format op2 socket local endpoint into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socknew`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x003e` | `{REG}` | Create a VM-managed TCP socket handle |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socknodelay`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004c` | `{REG,REG,REG}` | Set op2 socket TCP_NODELAY flag op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockpeer`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004e` | `{REG,REG}` | Format op2 socket peer endpoint into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockpending`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0049` | `{REG,REG}` | Query pending bytes on op2 socket handle into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockrecv`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0047` | `{REG,REG,REG}` | Receive up to op3 bytes from op2 socket handle as string into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockrecvb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0048` | `{REG,REG,REG}` | Receive up to op3 bytes from op2 socket handle as binary into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socksend`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0045` | `{REG,REG,REG}` | Send string op3 on op2 socket handle, bytes in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socksendb`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0046` | `{REG,REG,REG}` | Send binary op3 on op2 socket handle, bytes in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockshutdown`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0044` | `{REG,REG,REG}` | Shutdown op2 socket handle mode op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockstarttls`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0052` | `{REG,REG,REG}` | Start client TLS on op2 socket handle using host op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `sockstatus`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x0050` | `{REG,REG}` | Read last socket status for op2 into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `socktimeout`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x004a` | `{REG,REG,REG}` | Set op2 socket timeout in milliseconds op3, rc in op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `spawn`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01d2` | `{REG,REG,REG}` | Spawn Process op1 = exec op2 redirect op3 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `time`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01ca` | `{REG}` | Put time into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO

## `xtime`

Status: placeholder.

| Opcode | Operands | Assembler description |
| --- | --- | --- |
| `0x01cc` | `{REG,STRING}` | put special time properties into op1 |

Human reference content:

- Purpose: TODO
- Operand notes: TODO
- Result and side effects: TODO
- Signals/errors: TODO
- Example: TODO
- Related instructions: TODO
