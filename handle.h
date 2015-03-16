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