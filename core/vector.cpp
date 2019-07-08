#include "vector.h"

template <typename T>
void VectorImpl<T>::remove(int p_index) {
	std::vector<T>::erase(std::vector<T>::begin() + p_index);
}

template <typename T>
T *VectorImpl<T>::ptrw() {
	return std::vector<T>::data();
}

template <typename T>
const T *VectorImpl<T>::ptr() const {
	return std::vector<T>::data();
}

template <typename T>
T VectorImpl<T>::get(int p_index) {
	return std::vector<T>::operator[](p_index);
}

template <typename T>
const T VectorImpl<T>::get(int p_index) const {
	return std::vector<T>::operator[](p_index);
}

template <typename T>
void VectorImpl<T>::set( int p_index, const T &p_elem) {
	std::vector<T>::operator[](p_index) = p_elem;
}

template <typename T>
void VectorImpl<T>::insert( int p_pos, const T &p_val) {
	std::vector<T>::insert(std::vector<T>::begin() + p_pos, p_val);
}

template <typename T>
int VectorImpl<T>::find(const T &p_val, int p_from) const {
	for ( int a = 0; a < static_cast<int>(std::vector<T>::size()); a++) {
		if (p_val == std::vector<T>::operator[](a)) {
			return a;
		}
	}
	return -1;
}

template <typename T>
void VectorImpl<T>::sort() {
	std::sort(std::vector<T>::begin(), std::vector<T>::end());
}

template <typename T>
template <typename C>
void VectorImpl<T>::sort_custom() {
	C comparator;
	std::sort(std::vector<T>::begin(), std::vector<T>::end(), comparator);
}

template <typename T>
void VectorImpl<T>::ordered_insert(const T &p_val) {
	 int i = 0u;
	for (; i < std::vector<T>::size(); i++) {

		if (p_val < std::vector<T>::operator[](i)) {
			break;
		};
	};
	std::vector<T>::insert(i, p_val);
}

template <class T>
void VectorImpl<T>::invert() {
	std::reverse(std::vector<T>::begin(), std::vector<T>::end());
}

template <class T>
void VectorImpl<T>::append_array(const VectorImpl<T> &p_other) {
	for (const T &a : p_other.write) {
		std::vector<T>::push_back(a);
	}
}