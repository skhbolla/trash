# TRASH (TRacing SHell)
A minimalist POSIX style shell built to trace every syscall, hex-dump every buffer, and show you exactly how Unix works under the hood. This project implements the core mechanics of a command-line interface, including process creation via fork/exec, pipeline management with pipe, history persistence and autocompletion. Designed as a deep-dive into how the kernel manages user-space execution environments.

Refer [IEEE 1003.1 standard](https://pubs.opengroup.org/onlinepubs/9699919799/utilities/V3_chap02.html)
