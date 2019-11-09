Linux kernel
============

There are several guides for kernel developers and users. These guides can
be rendered in a number of formats, like HTML and PDF. Please read
[Documentation/admin-guide/README.rst](https://github.com/googlestadia/kernel/blob/stadia/linux-4.19.y/Documentation/admin-guide/README.rst) first.

In order to build the documentation, use ``make htmldocs`` or
``make pdfdocs``.  The formatted documentation can also be read online at:

    https://www.kernel.org/doc/html/latest/

There are various text files in the Documentation/ subdirectory,
several of them using the Restructured Text markup notation.
See [Documentation/00-INDEX](https://github.com/googlestadia/kernel/blob/stadia/linux-4.19.y/Documentation/00-INDEX) for a list of what is contained in each file.

Please read the [Documentation/process/changes.rst](https://github.com/googlestadia/kernel/blob/stadia/linux-4.19.y/Documentation/process/changes.rst) file, as it contains the
requirements for building and running the kernel, and information about
the problems which may result by upgrading your kernel.
