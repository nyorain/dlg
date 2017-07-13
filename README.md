dlg
===

Just another (soon-to-be-)lightweight logging library for c++.
Uses a slightly stripped version libfmt at the moment.
Name stands for some kind of super clever word mixture of the words 'debug' and 'log'.
Correctly outputs color (even on windows; oh my) and supports utf-8 in windows terminals.

Ideas
=====

Most of these points are not really important. Just a list of ideas.
The major thing left to do is building/refactoring the fmt source.

- [ ] rework/further strip fmt.hpp
	- [ ] since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] constexpr string parsing
		- [ ] warn about format issues
		- [ ] warn about unused but passed variables
- [x] windows utf-8 output (see ny)
- [x] windows text style support
- [ ] add field to Origin that determines whether the origin is inside a dlg::check block?
- [ ] assert without error message
- [ ] add real example and screenshot
- [ ] custom (changeable) base paths (for nested projects/header calls)
- [ ] variable Src<I> count
- [ ] possibility to get current scope (or more general: exception support)
- [x] make default scope signs customizable by macro
- [ ] beautiful base color configuration
