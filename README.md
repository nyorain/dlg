dlg
===

Just another (soon-to-be-)lightweight logging library for c++.
Uses a slightly stripped version libfmt at the moment.
Name stands for some kind of super clever word mixture of the words 'debug' and 'log'.
Not real windows support yet.

To do
=====

- [ ] add field to Origin that determines whether the origin is inside a dlg::check block?
- [ ] rework/further strip fmt.hpp
	- [ ] since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] constexpr string parsing
- [ ] windows utf-8 output (see ny)
- [ ] windows text style support
- [ ] assert without error message
- [ ] add real example and screenshot
- [ ] custom (changeable) base paths (for nested projects/header calls)
- [ ] variable Src<I> count
