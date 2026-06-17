# rxdb - the cRexx Debugger

The debugger is one of the programs in the toolchain delivered with cRexx
as its source code; most other programs, at the moment, are compiled
from C. It is easily adaptable and can be regarded a *debugger
  construction set*. By adapting and recompiling the user can
implement their own wishes for a debugger. In this sense, it can be
seen as an open-ended complement to the Rexx `trace`
statement. Because it has modes for Rexx as well as
`rxas` Assembler, it is a useful tool for debugging
high-level as well as low-level problems.

## Command Line Options

\fontspec{IBM Plex Mono}
\begin{terminaloutput}
\small
\obeylines \splice{rxdb -h | sed 's/\&/\\\&/g'}
\end{terminaloutput}
\fontspec{TeX Gyre Pagella}

## Runtime Options

After the `rxdb` program is started, a few runtime options appear in the
delivered version. This is an example session:

