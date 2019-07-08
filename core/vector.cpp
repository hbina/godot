#include "vector.h"

template <typename T>
void Vector<T>::remove(int p_index) {
	write.erase(write.begin() + p_index);
}

template <typename T>
void Vector<T>::erase(const T &p_val) {
	int idx = find(p_val);
	if (idx >= 0) remove(idx);
}

template <typename T>
T *Vector<T>::ptrw() {
	return write.data();
}

template <typename T>
const T *Vector<T>::ptr() const {
	return write.data();
}

template <typename T>
void Vector<T>::clear() {
	write.clear();
}

template <typename T>
bool Vector<T>::empty() const {
	return write.empty();
}

template <typename T>
T Vector<T>::get(int p_index) {
	return write[p_index];
}

template <typename T>
const T Vector<T>::get(int p_index) const {
	return write[p_index];
}

template <typename T>
void Vector<T>::set(unsigned int p_index, const T &p_elem) {
	CRASH_BAD_INDEX(p_index, write.size());
	write[p_index] = p_elem;
}

template <typename T>
int Vector<T>::size() const {
	return write.size();
}

template <typename T>
void Vector<T>::resize(int p_size) {
	write.resize(p_size);
}

template <typename T>
void Vector<T>::insert(unsigned int p_pos, const T &p_val) {
	write.insert(write.begin() + p_pos, p_val);
}

template <typename T>
int Vector<T>::find(const T &p_val, int p_from = 0) const {
	for (signed int a = 0; a < static_cast<signed int>(write.size()); a++) {
		if (p_val == write[a]) {
			return a;
		}
	}
	return -1;
}

template <typename T>
void Vector<T>::sort() {
	std::sort(write.begin(), write.end());
}

template <typename T>
template <typename C>
void Vector<T>::sort_custom() {
	C comparator;
	std::sort(write.begin(), write.end(), comparator);
}

template <typename T>
void Vector<T>::ordered_insert(const T &p_val) {
	unsigned int i = 0u;
	for (; i < write.size(); i++) {

		if (p_val < write[i]) {
			break;
		};
	};
	insert(i, p_val);
}

template <class T>
void Vector<T>::invert() {
	std::reverse(std::begin(write), std::end(write));
}

template <class T>
void Vector<T>::append_array(const Vector<T> &p_other) {
	for (const T &a : p_other.write) {
		write.push_back(a);
	}
}