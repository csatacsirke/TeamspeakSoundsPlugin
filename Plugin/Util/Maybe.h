#pragma once


#include <memory>


//template<class T>
//std::unique_ptr<T> as_unqiue(const T& value) {
//	return make_unique<T>(value);
//}
//
//template<class T>
//std::unique_ptr<T> as_unqiue(const T&& value) {
//	return make_unique<T>(value);
//}

//
//
//
//template<class T>
//class Maybe {
//	bool valid = false;
//	T value = T();
//public:
//
//	Maybe(const T& validValue) {
//		value = validValue;
//		valid = true;
//	}
//
//	Maybe(T&& validValue) {
//		value = validValue;
//		valid = true;
//	}
//
//	operator bool() const {
//		return valid;
//	}
//
//	T operator .() {
//		return value;
//	}
//
//};


