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

    $ clang -Xclang -load -Xclang build/skeleton/libNullDereferenceDetection.* examples/hello.c

[1]: https://www.cs.cornell.edu/~asampson/blog/llvm.html
[2]: https://github.com/sampsyo/llvm-pass-skeleton


# Possible null detections

- Null detection for local variables. If a variable with a pointer type has 0
  assigned to it, then track it and warn when dereferenced.
- Null detection for arguments. Track values of variables in a larger scope. 
- Detect `null` check: stop complaining if a `null` check has been done.
- Track `maybe` null return values. Analyse a function implementation to see if
  there exists a possibility of null.
