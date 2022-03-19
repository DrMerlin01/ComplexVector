#pragma once
#include <cassert>
#include <cstdlib>
#include <new>
#include <utility>
#include <memory>
#include <algorithm>

template <typename T>
class RawMemory {
public:
	RawMemory() = default;

	explicit RawMemory(size_t capacity)
		: buffer_(Allocate(capacity))
		, capacity_(capacity) {
	}

	RawMemory(const RawMemory&) = delete;

	RawMemory& operator=(const RawMemory& rhs) = delete;

	RawMemory(RawMemory&& other) noexcept { 
		Swap(other);
	}

	RawMemory& operator=(RawMemory&& rhs) noexcept {
		if(this != &rhs) {
			Swap(rhs); 
		}

		return *this;
	}

	~RawMemory() {
		Deallocate(buffer_);
	}

	T* operator+(size_t offset) noexcept {
		// Разрешается получать адрес ячейки памяти, следующей за последним элементом массива
		assert(offset <= capacity_);

		return buffer_ + offset;
	}

	const T* operator+(size_t offset) const noexcept {
		return const_cast<RawMemory&>(*this) + offset;
	}

	const T& operator[](size_t index) const noexcept {
		return const_cast<RawMemory&>(*this)[index];
	}

	T& operator[](size_t index) noexcept {
		assert(index < capacity_);
		
		return buffer_[index];
	}

	void Swap(RawMemory& other) noexcept {
		std::swap(buffer_, other.buffer_);
		std::swap(capacity_, other.capacity_);
	}

	const T* GetAddress() const noexcept {
		return buffer_;
	}

	T* GetAddress() noexcept {
		return buffer_;
	}

	size_t Capacity() const {
		return capacity_;
	}

private:
	// Выделяет сырую память под n элементов и возвращает указатель на неё
	static T* Allocate(size_t n) {
		return n != 0 ? static_cast<T*>(operator new(n * sizeof(T))) : nullptr;
	}

	// Освобождает сырую память, выделенную ранее по адресу buf при помощи Allocate
	static void Deallocate(T* buf) noexcept {
		operator delete(buf);
	}

	T* buffer_ = nullptr;
	size_t capacity_ = 0;
};

template <typename T>
class Vector {
public:
	using iterator = T*;
	using const_iterator = const T*;

	Vector() = default;

	explicit Vector(size_t size)
		: data_(size)
		, size_(size) {
		std::uninitialized_value_construct_n(begin(), size);
	}

	Vector(const Vector& other)
		: data_(other.size_)
		, size_(other.size_) {
		if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
			std::uninitialized_move_n(other.begin(), Size(), begin());
		} else {
			std::uninitialized_copy_n(other.begin(), Size(), begin());
		}
	}

	Vector(Vector&& other) noexcept {
		Swap(other);
	}

	Vector& operator=(const Vector& rhs) {
		if (this != &rhs) {
			if (rhs.Size() > data_.Capacity()) {
				Vector rhs_copy(rhs);
				Swap(rhs_copy);
			} else {
				std::copy_n(rhs.begin(), Size() < rhs.Size() ? Size() : rhs.Size(), begin());
				if (Size() <= rhs.Size()) {
					std::uninitialized_copy_n(rhs.begin() + Size(), rhs.Size() - Size(), begin());
				} else if (Size() > rhs.Size()) {
					std::destroy_n(begin() + rhs.Size(), Size() - rhs.Size());
				}
				size_ = rhs.Size();
			}
		}

		return *this;
	}

	Vector& operator=(Vector&& rhs) noexcept {
		if (this != &rhs) {
			Swap(rhs);
		}

		return *this;
	}

	~Vector() {
		std::destroy_n(begin(), Size());
	}

	void Swap(Vector& other) noexcept {
		data_.Swap(other.data_);
		std::swap(size_, other.size_);
	}

	void Reserve(size_t new_capacity) {
		if (new_capacity <= Capacity()) {
			return;
		}

		RawMemory<T> new_data(new_capacity);
		UninitializedArray(begin(), Size(), new_data.GetAddress());
		std::destroy_n(begin(), Size());
		data_.Swap(new_data);
	}

	void Resize(size_t new_size) {
		if (new_size < Size()) {
			std::destroy_n(begin() + new_size, Size() - new_size);
		} else {
			Reserve(new_size);
			std::uninitialized_value_construct_n(begin() + Size(), new_size - Size());
		}
		size_ = new_size;
	}

	iterator begin() noexcept {
		return data_.GetAddress();
	}

	iterator end() noexcept {
		return data_.GetAddress() + Size();
	}

	const_iterator begin() const noexcept {
		return data_.GetAddress();
	}

	const_iterator end() const noexcept {
		return data_.GetAddress() + Size();
	}

	const_iterator cbegin() const noexcept {
		return begin();
	}

	const_iterator cend() const noexcept {
		return end();
	}

	template <typename... Args>
	iterator Emplace(const_iterator pos, Args&&... args) {
		const size_t distance = pos - begin();
		if (Size() == Capacity()) {
			RawMemory<T> new_data(Size() == 0 ? 1 : Size() * 2);
			new (new_data + distance) T(std::forward<Args>(args)...);
			if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
				try {
					std::uninitialized_move_n(begin(),  distance, new_data.GetAddress());
				} catch (...) {
					std::destroy_n(begin() + distance, 1);
				}
				try {
					std::uninitialized_move_n(begin() + distance, Size() - distance, new_data.GetAddress() + distance + 1);
				} catch (...) {
					std::destroy_n(begin(), distance + 1);
				}
			} else {
				try {
					std::uninitialized_copy_n(begin(), distance, new_data.GetAddress());
				} catch (...) {
					std::destroy_n(begin() + distance, 1);
				}
				try {
					std::uninitialized_copy_n(begin() + distance, Size() - distance, new_data.GetAddress() + distance + 1);
				} catch (...) {
					std::destroy_n(begin(), distance + 1);
				}
			}
			std::destroy_n(begin(), Size());
			data_.Swap(new_data);
		} else if (Size() == 0) {
			std::move_backward(begin() + distance, end(), end() + 1);
			new (data_ + distance) T(std::forward<Args>(args)...);
		} else {
			T tmp(std::forward<Args>(args)...);
			std::move_backward(begin() + distance, end(), end() + 1);
			new (data_ + distance) T(std::move(tmp));
		}
		++size_;

		return begin() + distance;
	}

	iterator Erase(const_iterator pos) noexcept(std::is_nothrow_move_assignable_v<T>) {
		const size_t distance = pos - begin();
		std::move(begin() + distance + 1, end(), begin() + distance);
		PopBack();

		return begin() + distance; 
	}

	iterator Insert(const_iterator pos, const T& value) {
		return Emplace(pos, value);
	}

	iterator Insert(const_iterator pos, T&& value) {
		return Emplace(pos, std::move(value));
	}

	template <typename... Args>
	T& EmplaceBack(Args&&... args) {
		if (Size() == Capacity()) {
			RawMemory<T> new_data(Size() == 0 ? 1 : Size() * 2);
			new (new_data + Size()) T(std::forward<Args>(args)...);
			UninitializedArray(begin(), Size(), new_data.GetAddress());
			std::destroy_n(begin(), Size());
			data_.Swap(new_data);
		} else {
			new (data_ + Size()) T(std::forward<Args>(args)...);
		}
		++size_;

		return data_[Size() - 1];
	}

	void PushBack(const T& value) {
		EmplaceBack(value);
	}

	void PushBack(T&& value) {
		EmplaceBack(std::move(value));
	}

	void PopBack() noexcept {
		std::destroy_n(begin() + Size() - 1, 1);
		--size_;
	}

	size_t Size() const noexcept {
		return size_;
	}

	size_t Capacity() const noexcept {
		return data_.Capacity();
	}

	const T& operator[](size_t index) const noexcept {
		return const_cast<Vector&>(*this)[index];
	}

	T& operator[](size_t index) noexcept {
		assert(index < Size());
		return data_[index];
	}

private:
	RawMemory<T> data_;
	size_t size_ = 0;

	void UninitializedArray(T* in, const size_t size, T* out) {
		if constexpr (std::is_nothrow_move_constructible_v<T> || !std::is_copy_constructible_v<T>) {
			std::uninitialized_move_n(in, size, out);
		} else {
			std::uninitialized_copy_n(in, size, out);
		}
	}
};