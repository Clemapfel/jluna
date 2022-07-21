jluna: A Julia Wrapper for C++
==============================

Welcome to the documentation for jluna, which was designed and implemented by `C. Cords <https://clemens-cords.com>`_.

jluna allows for easy integration of Julia scripts and packages into projects with C++ as the
host language, making multi-language interaction easy and convenient.

It is available, for free, `on GitHub <https://github.com/Clemapfel/jluna>`_, under MIT License.

This documentation includes a step-by-step guide on how to install jluna, a manual and tutorial introducing all of jlunas
features, as well as an index of all of jlunas function, intended to be easily referenced back to during development.

--------------

.. note::
    For all chaptes, an interactive **table of contents is provided in the top right corner of the page**.

Table of Content
****************

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

With C++ being as popular as it is, confusion may arise as to what purpose jluna serves when compared to a number of
other C++-interfacing Julia packages. These will be addressed in this section.

Julia C-API
^^^^^^^^^^^

The Julia C-API provides the "backbone" of Julia and is the basis for jluna and most other foreign-language libraries
interacting with Julia. While powerful, the C-API can be quite hard to use and poorly documented. This fact was the
entire point of jluna, it aims to fully wrap the C-API, making it functionally equivalent but cushioned in all the
convenience of modern C++, such as automatic memory management, proper exception handling and much nicer syntax.

The one area where jluna actually has more features than the C-API is for parallelization, jluna provides a thread pool
that allows C-side functions to interact with the Julia state in paralell, something that is TODO LINK: not possible using only he
C-API.


`Cxx.jl <https://github.com/JuliaInterop/Cxx.jl>`_
^^^^^^

Cxx.jl aims to provide users with a way to run C++ code natively in Julia, using the :code:`@cxx` macro. Unlike jluna,
Cxx.jl assumes Julia as the host language, while jluna assumes C++. Futhermore, Cxx.jl is currently inactive, and only
supports older Julia versions < 1.3, while jluna specifically requires Julia 1.7 or newer.

`CxxWrap.jl <https://github.com/JuliaInterop/CxxWrap.jl>`_
^^^^^

CxxWrap.jl is related to Cxx.jl, in that both assume Julia as the host language and both allow for direct access of
C++ functionality from within Julia. It allows to compile C++ code into a shared library that can then be used directly
from within Julia. This is very similar to jluna, with two major difference. Firstly, jluna is a Julia-wrapper for C++,
while CxxWrap is a C++-wrapper for Julia. While in concept both overlap, the decision on which to use should be based on
what host language a specific project has. If Julia is the "main" language that interacts with all parts of a project, CxxWrap.jl
may be a better fit than jluna, if C++ takes that place, jluna may be superior.

In addition to this fundamental difference, CxxWrap.jl and jluna have no architecture or design in common, jluna was created
completely independently and does not mimic any of CxxWrap.jl features directly. Whether this is advantageous or disadvantageous
depends on the individual user and application.

Lastly, a much more minor difference is that jluna was designed from the ground up using C++20 techniques such as concepts,
making it's C++ code much more modern. In return for the nicer syntax and future-proofing, jluna requires up-to-date compilers,
a trade-off that the designers of jluna found worth it.

`CxxInterface.jl <https://juliahub.com/ui/Packages/CxxInterface/5a4dz/1.0.1>`_ / `CxxCall.jl <https://github.com/jw3126/CxxCall.jl>`_
^^^^^^

CxxInterface.jl and CxxCall.jl both aim to call C++ functions contained in a shared library from Julia. They essentially
are a `ccall` for C++, not a full language wrapper. This makes their and jluna funtionality overlap only slightly, and while
all three projects deal with Julia <-> C++ interoperability, they are not functionally equivalent. jluna does provide a TODO LINK:
way for the julia state to call any C++ function, though the mechanism to achieve this is completely different.

-----

In summary, the only functional equivalent for jluna is the Julia C-API, as only it and jluna provide a Julia-wrapper that assumes
C++ as the host language. It is therefore the superior choice in projects where Julia plays an auxiliary rule, and jlunas
clearer design, documentation and modernity make it easier to use than the C-API.




