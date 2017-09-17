# Ideas and todo

- [x] windows utf-8 output (see ny)
- [x] windows text style support
- [x] assert without error message
- [x] add real example and screenshot
- [x] custom (changeable) base paths (for nested projects/header calls) __[DLG_FILE]__
- [x] possibility to get current scope (or more general: exception support)
- [x] make default scope signs customizable by macro
- [ ] update picture for tags (and C) update
- [x] unit tests (at least some basic stuff) + ci (travis)
- [ ] assert_failed function (maybe as c symbol) that can be easily used as breakpoint
	- [ ] Also custom assertion handler? that is called inline and might throw?
- [ ] example for custom failed assertion handle, i.e. print backtrace/exception/abort
- [x] decide on whether to catch exceptions from assert expressions. Config variable?
	- Yeah, don't do it. We are c now

### Make probably no sense:

- [ ] rework/further strip fmt.hpp
	- [ ] since it is parsed to some type-erased list anyways, don't include the whole header
	- [ ] ~~constexpr string parsing~~ __[Not really worth it/fully possible i guess]__
		- [ ] ~~warn about format issues~~
		- [ ] ~~warn about unused but passed variables~~
- [ ] add field to Origin that determines whether the origin is inside a dlg::check block?
- [ ] add at least really simply pattern matching utility function for tags?
- [ ] make dlg_assert return false on failure

- maybe put the output stuff in a different namespace? dlg::output
- tag order? make it modifiable or define it somewhere at least? should they be used in an ordered way?
