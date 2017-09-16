// Copyright (c) 2017 nyorain
// Distributed under the Boost Software License, Version 1.0.
// See accompanying file LICENSE or copy at http://www.boost.org/LICENSE_1_0.txt

#ifndef _DLG_DLG_HPP_
#define _DLG_DLG_HPP_

#include <dlg/dlg.h>

namespace dlg {

class TagsGuard {
public:
	TagsGuard(const char** tags, const char* func) : tags_(tags), func_(func) {
		while(tags++) {
			dlg_add_tag(*tags, func);
		}
	}

	~TagsGuard() {
		while(tags_++) {
			dlg_remove_tag(*tags_, func_);
		}
	}

protected:
	const char** tags_;
	const char* func_;
};

#ifdef DLG_DISABLE
	#define dlg_tags() 
	#define dlg_tags_global()
#else
	#define dlg_tags(...) \
		::dlg::TagsGuard _dlgltg_((const char* tags[]){__VA_ARGS__}, __func__)
	#define dlg_tags_global(...) \
		::dlg::TagsGuard _dlggtg_((const char* tags[]){__VA_ARGS__}, nullptr)
#endif


} // namespace dlg

#endif // header guard