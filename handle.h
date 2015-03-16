/*
Copyright (c) 2015 Dilyan Rusev

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
THE SOFTWARE.
*/

#ifndef FOO_ASTEROIDS_HANDLE_H_
#define FOO_ASTEROIDS_HANDLE_H_

#include <utility>

namespace foo {

template <typename Traits>
class Handle {
	Traits traits_;
	bool initialized_;

public:
	template<typename... Args>
	Handle(Args&&... args) : initialized_(false) {
		traits_.Create(std::forward<Args>(args)...);
		initialized_ = true;
	}

	Handle(const Handle&) = delete;
	Handle(Handle&& other) : initialized_(false) {
		std::swap(traits_, other.traits_);
		std::swap(initialized_, other.initialized_);
	}

	~Handle() {
		if (!initialized_) return;

		traits_.Destroy();
	}

	Handle& operator=(const Handle&) = delete;
	Handle& operator=(Handle&& other) {
		if (this != &other) {
			if (initialized_) {
				traits_.Destroy();
			}
			std::swap(traits_, other.traits_);
			initialized_ = other.initialized_;
			other.initialized_ = false;
		}
		return *this;
	}

	template<typename... Args>
	void Create(Args&&... args) {
		if (initialized_) {
			traits_.Destroy();
		}
		traits_.Create(std::forward<Args>(args)...);
		initialized_ = true;
	}

	void Destroy() {
		traits_.Destroy();
	}
};

}

#endif //FOO_ASTEROIDS_HANDLE_H_