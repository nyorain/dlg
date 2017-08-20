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

Thousands words, explanations and pictures don't say as much as a single __[code example](doc/example.cpp)__.

Nontheless a rather beautiful picture of dlg in action for you. It is probably rather nonsensical without
having read the example though:

![Here should a beautiful picture of dlg in action be erected. What a shame!](doc/example_crop.png)

Note though that dlg can be used without weird dummy messages as well.
Building the sample can be enabled by passing the 'sample' argument as true to
cmake or meson.

## Ideas and todo

- [x] windows utf-8 output (see ny)
- [x] windows text style support
- [x] assert without error message
- [x] add real example and screenshot
- [x] custom (changeable) base paths (for nested projects/header calls) __[DLG_FILE]__
- [x] possibility to get current scope (or more general: exception support)
- [x] make default scope signs customizable by macro
- [ ] update picture for tags update
- [ ] unit tests (at least some basic stuff)

Make probably no sense:

- [ ] rework/further strip fmt.hpp
	- [ ] since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] ~~constexpr string parsing~~ __[Not really worth it/fully possible i guess]__
		- [ ] ~~warn about format issues~~
		- [ ] ~~warn about unused but passed variables~~
- [ ] add field to Origin that determines whether the origin is inside a dlg::check block?

- maybe put the output stuff in a different namespace? dlg::output


__Contributions of all kind are welcome, this is nothing too serious though (kinda like life)__
