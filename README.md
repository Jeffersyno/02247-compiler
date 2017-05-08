# 02247 Compilers Project

We used the llvm pass skeleton from [here][1] cloned from [here][2].
It is released under the MIT license. See [SKELETON-LICENSE](./SKELETON-LICENSE).

Build:

    $ mkdir build
    $ cd build
    $ cmake ..
    $ make
    $ cd ..

Run:

    # Run manually:
    $ clang -Xclang -load -Xclang build/skeleton/libNullDereferenceDetection.* examples/hello.c

    # Or use one of the bash scripts:
    $ ./opt <folder>/<example>            # compile with clang and run with opt
    $ ./compile <folder>/<example>        # compile and run pass with clang

[1]: https://www.cs.cornell.edu/~asampson/blog/llvm.html
[2]: https://github.com/sampsyo/llvm-pass-skeleton


# Possible null detections

- Null detection for local variables within a function. If a variable with a
  pointer type has 0 assigned to it, then track it and warn when dereferenced.
  (done)
- Detect `null` check: stop complaining if a `null` check has been done.
  (interesting, but not done)
- Track `maybe` null return values. Analyse a function implementation to see if
  there exists a possibility of null. (done, but no warnings)
