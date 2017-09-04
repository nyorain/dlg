dlg
===

Just another (at-the-moment-not-so-much-yet-but-pretty-alright-i-guess-)lightweight logging library
for C++ 17 (which implies that it will probably not build on msvc).
Uses a slightly stripped version of libfmt at the moment (that is pretty much all of its 'bloat').
The name stands for some kind of super clever word mixture of the words 'debug' and 'log' (think
of something yourself, duh).

Can be built with cmake or meson (please consider jumping onto the meson train though,
at some point i may no longer want to maintain the cmake version).

## Show me something already

Thousands words, explanations and pictures don't say as much as a single __[code example](docs/example.cpp)__.

Nontheless a rather beautiful picture of dlg in action for you. It is probably rather nonsensical without
having read the example though:

![Here should a beautiful picture of dlg in action be erected. What a shame!](docs/example.png)

Note though that dlg can be used without weird dummy messages as well.
Building the sample can be enabled by passing the 'sample' argument as true to meson.

__Contributions of all kind are welcome, this is nothing too serious though (kinda like life)__
