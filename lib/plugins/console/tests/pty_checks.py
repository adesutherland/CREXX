"""Real foreground-PTY tests. No fixed startup sleeps; output is the rendezvous."""
import argparse
import errno
import fcntl
import os
import pathlib
import select
import signal
import struct
import subprocess
import termios
import time


class Terminal:
    def __init__(self, command, cwd, hold=True):
        self.master, self.slave = os.openpty()
        self.before = termios.tcgetattr(self.slave)
        self.resize(80, 24)
        self.output = b""
        self.cursor = 0

        def foreground():
            os.setsid()
            fcntl.ioctl(0, termios.TIOCSCTTY, 0)
            if not hold:
                signal.signal(signal.SIGHUP, signal.SIG_IGN)

        env = dict(os.environ, TERM="xterm-256color")
        if hold:
            # Keep the controlling session alive after the VM exits. On macOS
            # a dead session leader makes tcgetattr(slave) fail with ENOTTY.
            # This also models returning to a real interactive shell.
            command = ["/bin/sh", "-c",
                       '"$@"; status=$?; printf "\\nPTY_EXIT:%s\\n" "$status"; '
                       'read -r release; exit "$status"', "rxconsole-pty"] + command
        self.process = subprocess.Popen(command, cwd=cwd, env=env,
                                        stdin=self.slave, stdout=self.slave,
                                        stderr=self.slave, preexec_fn=foreground)

    def resize(self, columns, rows):
        fcntl.ioctl(self.slave, termios.TIOCSWINSZ,
                    struct.pack("HHHH", rows, columns, 0, 0))

    def send(self, data):
        os.write(self.master, data)

    def read(self, timeout=0.1):
        if select.select([self.master], [], [], timeout)[0]:
            try:
                data = os.read(self.master, 65536)
            except OSError as error:
                if error.errno == errno.EIO:
                    return
                raise
            self.output += data
            if len(self.output) > 4 * 1024 * 1024:
                raise AssertionError("unbounded terminal output")

    def expect(self, text, timeout=20):
        deadline = time.monotonic() + timeout
        while time.monotonic() < deadline:
            index = self.output.find(text, self.cursor)
            if index >= 0:
                self.cursor = index + len(text)
                return
            self.read()
            if self.process.poll() is not None:
                self.read(0)
                if text not in self.output[self.cursor:]:
                    break
        raise AssertionError(f"missing {text!r}; exit={self.process.poll()}; tail={self.output[-3000:]!r}")

    def finish(self):
        self.expect(b"PTY_EXIT:0")
        assert termios.tcgetattr(self.slave) == self.before, "terminal modes not restored"
        assert b"\x1b[?1049l" in self.output, "alternate screen not restored"
        assert b"\x1b[?25h" in self.output, "cursor not restored"
        self.send(b"\n")
        deadline = time.monotonic() + 15
        while self.process.poll() is None and time.monotonic() < deadline:
            self.read()
        assert self.process.poll() == 0, self.output[-3000:]
        self.read(0)

    def close(self):
        if self.process.poll() is None:
            os.killpg(self.process.pid, signal.SIGKILL)
            self.process.wait()
        if self.master >= 0:
            os.close(self.master)
        os.close(self.slave)


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--build", required=True, type=pathlib.Path)
    parser.add_argument("--source", required=True, type=pathlib.Path)
    parser.add_argument("--mode", choices=["console", "app", "host-edges"], required=True)
    parser.add_argument("--variant", choices=["noopt", "opt"], default="opt")
    args = parser.parse_args()
    build, source = args.build.resolve(), args.source.resolve()
    binary = build / "bin"
    vm = str(binary / "rxvm")
    if args.mode == "console":
        program = build / "lib/plugins/console/console_pty.rxbin"
        for mode in ("normal", "abandon"):
            terminal = Terminal([vm, str(program), str(binary / "library"), "-a", mode], source)
            try:
                terminal.expect(b"CONSOLE_READY")
                terminal.resize(91, 27)
                terminal.expect(b"EVENT 4 0 0 91 27")
                terminal.send(b"\x1b[<4;12;7M")
                terminal.expect(b"EVENT 5 11 6 0 0 1 1")
                terminal.send(b"\x1b[<4;12;7m")
                terminal.expect(b"EVENT 5 11 6 0 0 1 2")
                terminal.send(b"\x1b[200~q is paste\x1b[201~")
                terminal.expect(b"EVENT 3 0 0 0 0 0 0 q is paste")
                terminal.send(b"\xc3\xa9")
                terminal.expect(b"EVENT 2 0 0 0 0 0 0 \xc3\xa9")
                terminal.send(b"q")
                terminal.finish()
            finally:
                terminal.close()
        # Exercise an actual vanished terminal, with SIGHUP ignored only in
        # this test so the public poll disconnect result can be observed.
        terminal = Terminal([vm, str(program), str(binary / "library"), "-a", "normal"], source, hold=False)
        try:
            terminal.expect(b"CONSOLE_READY")
            os.close(terminal.master)
            terminal.master = -1
            assert terminal.process.wait(timeout=10) == 0, "disconnect did not end the input loop"
        finally:
            terminal.close()
    elif args.mode == "host-edges":
        program = build / "lib/ui/tests_functional" / f"ui_ansi_edges_{args.variant}.rxbin"
        terminal = Terminal([vm, str(program)], source)
        try:
            terminal.expect(b"PASS: ANSI shared host edges")
            terminal.finish()
        finally:
            terminal.close()
    else:
        modules = [build / "examples/ui/text-inspector/text_inspector_ansi",
                   build / "examples/ui/text-inspector/text_inspector"]
        modules += [binary / name for name in ("ui_ansi", "ui_dialogs", "ui_terminal_view",
                    "ui_local_resources", "ui_contract", "ui_catalog", "ui", "library")]
        terminal = Terminal([vm] + [str(path) for path in modules], source)
        try:
            terminal.expect(b"Status: Ready")
            terminal.send(b"o")
            terminal.expect(b"Open a text file")
            terminal.resize(20, 5)
            terminal.expect(b"Resize terminal")
            terminal.resize(80, 24)
            terminal.expect(b"Open a text file")
            terminal.send(b"\x1b")
            terminal.expect(b"Open cancelled")
            terminal.send(b"o")
            terminal.expect(b"Open a text file")
            path = source / "examples/ui/text-inspector/fixtures/sample.txt"
            terminal.send(b"\x1b[200~" + str(path).encode() + b"\x1b[201~\r")
            terminal.expect(b"Count complete")
            terminal.send(b"c")
            terminal.expect(b"Clear results?")
            terminal.send(b"\x1b")
            terminal.expect(b"Clear cancelled")
            terminal.send(b"c")
            terminal.expect(b"Clear results?")
            # Ignored ordinary text must not move confirmation focus to the
            # file editor. Enter still operates the default OK button.
            terminal.send(b"ignored\r")
            terminal.expect(b"Status: Ready")
            terminal.send(b"c")
            terminal.expect(b"Clear results?")
            terminal.resize(40, 12)
            terminal.expect(b"Clear results?")
            # Click the rendered OK button (zero-based 2,4 => SGR 3,5).
            terminal.send(b"\x1b[<0;3;5M\x1b[<0;3;5m")
            terminal.expect(b"Status: Ready")
            terminal.send(b"q")
            terminal.finish()
        finally:
            terminal.close()
    print("PASS: real PTY", args.mode, "input, resize, dialogs and restoration")


if __name__ == "__main__":
    main()
