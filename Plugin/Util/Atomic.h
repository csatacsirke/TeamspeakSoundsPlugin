#pragma once


namespace TSPlugin {

	template<class T>
	class Atomic {
		mutable std::mutex mutex;
		T value;
	public:
		template<typename ... Args>
		Atomic(Args&& ... args) : value(std::forward<Args>(args)...) {
			// ...
		}

		Atomic(Atomic<T>&&) = delete;
		Atomic(const Atomic<T> &) = delete;

		template<class S>
		operator S() const {
			std::lock_guard lock(mutex);
			return value;
		}

		template<class S>
		auto& operator=(S&& newValue) {
			std::lock_guard lock(mutex);
			value = std::move(newValue);
			return *this;
		}

		template<class S>
		auto& operator=(const S& newValue) {
			std::lock_guard lock(mutex);
			value = newValue;
			return *this;

		}
		T Copy() const {
			std::lock_guard lock(mutex);
			return value;
		}
	};

}