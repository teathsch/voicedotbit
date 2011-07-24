// Copyright 2011 Heath Schultz et. al. under liberal licenses. See LICENSE for details.

// Use scopeguard idiom to do cleanups when exiting scope.

#ifndef SCOPEGUARD_H
#define SCOPEGUARD_H

#include <iostream>
#include <tr1/functional>

template<typename Bool_t> class scope_guard_t { public:
		 scope_guard_t(std::tr1::function<void()> f) : \
		                          released(false) , fun(f) {}
		~scope_guard_t() { try { fun(); } catch (...) {} }

	private:
		Bool_t released;
		std::tr1::function<void()> fun;

}; typedef scope_guard_t<bool> scope_guard;

#endif
