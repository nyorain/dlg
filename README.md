dlg
===

Just another (soon-to-be-)lightweight logging library for c++.
Uses a slightly stripped version libfmt at the moment.
Name stands for some kind of super clever word mixture of the words 'debug' and 'log'.
Not real windows support yet.

To do
=====

- [ ] Add field to Origin that determines whether the origin is inside a dlg::check block?
- [ ] Rework/further strip fmt.hpp
	- [ ] Since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] constexpr string parsing
- [ ] windows utf-8 output (see ny)
- [ ] windows text style support
