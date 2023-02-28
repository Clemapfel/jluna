jluna: A Julia Wrapper for C++
==============================

Welcome to the documentation for jluna, a Julia-wrapper for C++.

It allows for easy integration of Julia scripts and packages into projects with C++ as the
host language, making language interaction easy and convenient.

It is available under MIT License, `on GitHub <https://github.com/Clemapfel/jluna>`_.

This documentation includes a `step-by-step guide on how to install jluna <./installation.html>`_, a `manual and tutorial introducing all of jlunas
features <./basics.html>`_, as well as an `index of all of jlunas function <./list.html>`_, intended to be easily referenced back to during development.

jluna was designed and implemented by `C. Cords <https://clemens-cords.com>`_.

--------------

.. note::
    For all chapters, an interactive **list of sections is provided in the top right corner of the page**.

Table of Contents
*****************

Please navigate to the appropriate section below:

.. toctree::

    installation.md
    troubleshooting.md
    basics.md
    proxies.md
    arrays.md
    symbols.md
    modules.md
    cppcall.md
    types.md
    usertypes.md
    multi_threading.md
    unsafe.md
    benchmarks.md
    closing_statement.md
    list.rst

--------------------

FAQ: What is the difference between jluna and ...?
************************************************

With C++ being as widely used as it is, confusion may arise as to what purpose jluna serves when compared to a number of
other C++-interfacing Julia packages. These will be addressed in this section.

Julia C-API
^^^^^^^^^^^

The Julia C-API provides the "backbone" of Julia and is the basis for jluna and most other foreign-language libraries
interacting with Julia. While powerful, the C-API can be quite hard to use and poorly documented. jluna aims to resolve
this, fully wrapping the C-API in all of the convenience of modern C++: automatic memory management, proper exception
handling and much nicer syntax. Other than this, Julia C-API and jluna are functionally equivalent.

The one area where jluna actually has more features than the C-API is in parallelization: jluna provides a thread pool
that allows C-side functions to interact with the Julia state, something that is `not possible <./multi_threading.html>`_ using only he
C-API.

`Cxx.jl <https://github.com/JuliaInterop/Cxx.jl>`_
^^^^^^^

Cxx.jl aims to provide users with a way to run C++ code natively from within Julia, using the :code:`@cxx` macro. Unlike jluna,
Cxx.jl assumes Julia as the host language, while jluna assumes C++. Furthermore, the Cxx.jl is currently unmaintained, only
supporting Julia versions older than 1.3. jluna specifically requires 1.7 or newer.


`CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_
^^^^^

CxxWrap.jl is related to Cxx.jl, in that both assume Julia as the host language and both allow for direct access of
C++ functionality from within Julia. It allows to compile C++ code into a shared library that can then be used directly
from within Julia. This is very similar to jluna, with two major difference: Firstly, jluna is a Julia-wrapper for C++,
while CxxWrap is a C++-wrapper for Julia. While, in concept, both overlap, the decision on which to use should be based on
what host language a specific project has. If Julia is the "main" language that interacts with all parts of a project, CxxWrap.jl
may be a better fit than jluna, if C++ takes that place, jluna may be superior.

In addition, CxxWrap.jl and jluna have no architectural design in common, jluna was created
completely independently and does not mimic any of CxxWrap.jl features directly, and vice-versa.
Whether this is advantageous or disadvantageous depends on the individual user and application.

Lastly, a much more minor difference is that jluna was designed from the ground up using C++20 techniques such as concepts,
making it's C++ code much more modern. In exchange for the nicer syntax and future-proofing, jluna requires up-to-date compilers.

`CxxInterface.jl <https://juliahub.com/ui/Packages/CxxInterface/5a4dz/1.0.1>`_ / `CxxCall.jl <https://github.com/jw3126/CxxCall.jl>`_
^^^^^^^^^^^^^^^^

CxxInterface.jl and CxxCall.jl both aim to call C++ functions (contained in a shared library) from Julia. They essentially
are a `ccall` for C++, not a full language wrapper, the projects are therefore not functionally equivalent. jluna `does provide a
way for the julia state to call any C++ function <./cppcall.html>`_, the mechanism to achieve this is completely different, however.

-----

In summary, the only fully functional equivalent for jluna is the Julia C-API, as only it provides a Julia-wrapper that assumes
C / C++ as the host language. It is therefore the superior choice in projects where Julia plays an auxiliary rule while C++
is more dominant, as jluna provides clearer design, more documentation and overall ease-of-use when compared to the C-API.

----

FAQ: Is it done / fast yet?
***************************

jluna is feature-complete as of 0.9.0. This release also included `extensive benchmarking <./benchmarks.html>`_, quantifying
jlunas performance and proving its capability for excellent performance. Correctness is assured through automated testing.

1.0 released in February 2023, as of that release, Windows support is no longer experimental, and Windows machines are now fully supported.

-----

FAQ: Did you know Julia 1.9 will include foreign thread support?
****************************************************************

I am aware that with the release of Julia 1.9 basically the entire multi-threading module will have to be deprecated.
Until then, multi-threading should still be supported for less up-to-date machines, however, developers
using jluna should be aware that the multi-threading module (and only it) may be marked for deletion at some point. For all
other library features, jluna guarantees version continuity and backwards compatibility.






