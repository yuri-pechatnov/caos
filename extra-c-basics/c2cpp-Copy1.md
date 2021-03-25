



```python

```


```python

```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.34
    user 0.24
    sys 0.09



```python

```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.28
    user 0.23
    sys 0.04



```python

```


```python

```


```cpp
%%cpp cb.h

#ifndef DEQUE_CIRCULAR_BUFFER_H
#define DEQUE_CIRCULAR_BUFFER_H

#include "cstddef"
#include <iostream>
#include <cstdlib>
#include <cstring>

template <typename T>
class CircularBuffer {
private:
    size_t head_;
    size_t rear_;
    size_t size_;
    size_t capacity_;
    T **data_;

public:
    CircularBuffer() : head_(0), rear_(0), size_(0), capacity_(0), data_(nullptr) {
        // fprintf(stderr, "in def CB\n");
    }

    explicit CircularBuffer(size_t count) : head_(0), rear_(0), size_(0), capacity_(count) {
        data_ = new T *[count];
    }
    CircularBuffer(const CircularBuffer<T> &other)
        : head_(other.head_), rear_(other.rear_), size_(other.size_), capacity_(other.capacity_) {
        data_ = new T *[capacity_];
        for (size_t i = 0; i < size_; i++) {
            data_[(i + head_) % capacity_] = other.data_[(i + head_) % capacity_];
        }
    }
    CircularBuffer<T> &operator=(const CircularBuffer<T> &other) {
        if (this != &other) {
            // fprintf(stderr, "in cb copy\n");
            for (size_t i = 0; i < capacity_; i++) {
                delete data_[(head_ + i) % capacity_];
            }
            delete[] data_;
            size_ = other.size_;
            capacity_ = other.capacity_;
            rear_ = other.rear_;
            head_ = other.head_;

            if (capacity_ > 0) {
                data_ = new T *[capacity_];
                for (size_t i = 0; i < size_; i++) {
                    T tmp = *(other.data_[(head_ + i) % capacity_]);
                    data_[(head_ + i) % capacity_] = new T(tmp);
                }
                for (size_t i = size_; i < capacity_; i++) {
                    data_[(head_ + i) % capacity_] = new T;
                }
            } else {
                data_ = nullptr;
            }
        }
        return *this;
    }
    ~CircularBuffer() {
        for (size_t i = 0; i < capacity_; i++) {
            delete data_[(head_ + i) % capacity_];
        }
        delete[] data_;
    }

    void Reserve(size_t new_size) {
        if (new_size > capacity_) {
            capacity_ = capacity_ == 0 ? 1 : capacity_;
            // capacity will change so we need to shift all elements in our data.
            ShiftMem();
            capacity_ = new_size;
            T *tmpv = new T[capacity_];
            for (size_t i = 0; i < size_; i++) {
                tmpv[i] = data_[i];
            }
            delete[] data_;
            data_ = tmpv;
            head_ = 0;
            rear_ = size_;
        }
    }
    void UpdateMem() {
        if (size_ >= capacity_) {
            // fprintf(stderr, "in update\n");
            capacity_ = capacity_ == 0 ? 1 : capacity_;
            ShiftMem();
            capacity_ *= 2;
            T **tmpv = new T *[capacity_];
            for (size_t i = 0; i < size_; i++) {
                // fprintf(stderr, "in uodate cycle\n");
                tmpv[i] = data_[i];
            }
            for (size_t i = size_; i < capacity_; i++) {
                tmpv[i] = new T;
                }
            delete[] data_;
            data_ = tmpv;
            head_ = 0;
            rear_ = size_;
        }
    }
    void ShiftMem() {
        T **buff = new T *[capacity_];
        // fprintf(stderr, " in shift\n");
        for (size_t i = 0; i < size_; i++) {
            // fprintf(stderr, "first cycle in shift\n");
            head_ %= capacity_;
            buff[i] = data_[head_++];
        }
        for (size_t i = 0; i < size_; ++i) {
            // fprintf(stderr, "second cycle in shift\n");
            data_[i] = buff[i];
        }
        delete[] buff;
    }
    void PushBack() {
        // std::cout << "true\n";
        // fprintf(stderr, " before UpdateMem\n");
        UpdateMem();

        rear_ %= capacity_;
        rear_++;
        // data_[rear_++] = new T(elem);
        rear_ %= capacity_;
        ++size_;
        // fprintf(stderr, "%p - ptr in push back\n", &data_[0]);
    }

    void PushFront() {
        UpdateMem();
        // head will move "upside down" while pushing and conversely while popping
        head_ = (head_ + capacity_ - 1) % capacity_;
        // data_[head_] = new T(elem);
        ++size_;
    }

    T &Back() {
        return *(data_[(rear_ + capacity_ - 1) % capacity_]);
    }
    const T &Back() const {
        return *(data_[(rear_ + capacity_ - 1) % capacity_]);
    }
    T &operator[](size_t idx) {
        return *(data_[(head_ + idx) % capacity_]);
    }
    const T &operator[](size_t idx) const {
        return *(data_[(head_ + idx) % capacity_]);
    }
    T &Front() {
        return *(data_[head_ % capacity_]);
    }
    const T &Front() const {
        return *(data_[head_ % capacity_]);
    }

    T PopBack() {
        T tmp = Back();
        --size_;
        // it is like usual popping but so that it dependen from capacity
        rear_ = (rear_ + capacity_ - 1) % capacity_;
        return tmp;
    }

    T PopFront() {
        T tmp = Front();
        size_--;
        head_ = (head_ + 1) % capacity_;
        return tmp;
    }

    void Clear() {
        size_ = 0;
        head_ = 0;
        rear_ = 0;
    }
    bool Empty() const {
        return size_ == 0;
    }
    size_t Capacity() const {
        return capacity_;
    }
    size_t Size() const {
        return size_;
    }
    void Swap(CircularBuffer<T> &other) {
        std::swap(data_, other.data_);
        std::swap(capacity_, other.capacity_);
        std::swap(size_, other.size_);
        std::swap(head_, other.head_);
        std::swap(rear_, other.rear_);
    }
};

#endif  // DEQUE_CIRCULAR_BUFFER_H
```


```cpp
%%cpp page.h

#ifndef DEQUE_PAGE_PAGE_H
#define DEQUE_PAGE_PAGE_H

#include "cstddef"
#include <iostream>
#include <cstdlib>
#include <cstring>

template <class T>
class Page {
private:
    size_t head_;
    size_t rear_;
    size_t size_;
    size_t capacity_;
    bool frontal_;
    T *data_;

public:
    Page() : head_(0), rear_(0), size_(0), capacity_(100), frontal_(true) {
        data_ = new T[capacity_];
    }
    bool IsFront() const {
        if (size_ == capacity_) {
            return false;
        }
        return frontal_;
    }
    bool IsBack() const {
        if (size_ == 0) {
            return true;
        }
        if (size_ == capacity_) {
            return false;
        }
        return !frontal_;
    }
    Page(const Page<T> &other)
        : head_(other.head_)
        , rear_(other.rear_)
        , size_(other.size_)
        , capacity_(other.capacity_)
        , frontal_(other.frontal_) {
        data_ = new T[capacity_];
        for (size_t i = 0; i < size_; i++) {
            data_[(i + head_) % capacity_] = other.data_[(i + head_) % capacity_];
        }
    }
    Page<T> &operator=(const Page<T> &other) {
        if (this != &other) {
            // fprintf(stderr, "soidv\n");
            size_ = other.size_;
            capacity_ = other.capacity_;
            frontal_ = other.frontal_;
            head_ = other.head_;
            rear_ = other.rear_;
            delete[] data_;
            if (capacity_ > 0) {
                data_ = new T[capacity_];
                for (size_t i = 0; i < capacity_; i++) {
                    data_[(i + head_) % capacity_] = other.data_[(i + head_) % capacity_];
                }
            } /*else data_ = nullptr;*/
        }
        return *this;
    }
    ~Page() {
        delete[] data_;
    }
    void PushBack(T elem) {
        // UpdateMem();
        if (size_ == 0) {
            frontal_ = false;
        }
        rear_ %= capacity_;

        data_[rear_++] = elem;
        rear_ %= capacity_;
        ++size_;
    }

    void PushFront(T elem) {
        // UpdateMem();
        // head will move "upside down" while pushing and conversely while popping
        head_ = (head_ + capacity_ - 1) % capacity_;
        data_[head_] = elem;
        ++size_;
    }
    T &Back() {
        return data_[(rear_ + capacity_ - 1) % capacity_];
    }
    const T &Back() const {
        return data_[(rear_ + capacity_ - 1) % capacity_];
    }
    T &operator[](size_t idx) {
        return data_[(head_ + idx) % capacity_];
    }
    const T &operator[](size_t idx) const {
        return data_[(head_ + idx) % capacity_];
    }
    T &Front() {
        return data_[head_ % capacity_];
    }
    const T &Front() const {
        return data_[head_ % capacity_];
    }

    T PopBack() {
        T tmp = Back();
        --size_;
        // it is like usual popping but so that it dependen from capacity
        rear_ = (rear_ + capacity_ - 1) % capacity_;
        if (size_ == 0) {
            frontal_ = true;
        }
        return tmp;
    }

    T PopFront() {
        T tmp = Front();
        size_--;
        head_ = (head_ + 1) % capacity_;
        return tmp;
    }

    void Clear() {
        size_ = 0;
        head_ = 0;
        rear_ = 0;
        frontal_ = true;
    }
    bool Empty() const {
        return size_ == 0;
    }
    bool Full() const {
        return size_ == capacity_;
    }
    size_t Capacity() const {
        return capacity_;
    }
    size_t Size() const {
        return size_;
    }
};

#endif  // DEQUE_PAGE_PAGE_H
```


```cpp
%%cpp dq.h

#ifndef DEQUE_DEQUE_H
#define DEQUE_DEQUE_H

#include "page.h"
#include "cb.h"

const int kPageSize = 100;

template <class T>
class Deque {
private:
    size_t npages_;

public:
    CircularBuffer<Page<T>> data_;
    Deque() : /*data_(1), npages_(0)*/ npages_(0), data_() {
    }
    Deque(const Deque<T> &other) {
        data_ = other.data_;
        npages_ = other.npages_;
    }
    Deque<T> &operator=(const Deque<T> &other) {
        if (this != &other) {
            data_ = other.data_;
            npages_ = other.npages_;
        }
        return *this;
    }
    ~Deque() = default;
    size_t Size() const {
        if (npages_ == 0) {
            return 0;
        }
        if (npages_ == 1) {
            return data_.Front().Size();
        }
        return data_.Front().Size() + (npages_ - 2) * kPageSize + data_.Back().Size();
    }
    void Swap(Deque<T> &other) {
        data_.Swap(other.data_);
        std::swap(npages_, other.npages_);
    }
    T &operator[](size_t idx) {
        if (data_.Front().Size() > idx) {
            return data_.Front()[idx];
        }

        size_t back_strt_idx = data_.Front().Size() + (npages_ - 2) * kPageSize;
        if (idx >= back_strt_idx) {
            return data_.Back()[idx - back_strt_idx];
        }
        return data_[(idx - data_.Front().Size()) / kPageSize + 1][(idx - data_.Front().Size()) % kPageSize];
    }
    const T &operator[](size_t idx) const {
        if (data_.Front().Size() > idx) {
            return data_.Front()[idx];
        }

        size_t back_strt_idx = data_.Front().Size() + (npages_ - 2) * kPageSize;
        if (idx >= back_strt_idx) {
            return data_.Back()[idx - back_strt_idx];
        }
        return data_[(idx - data_.Front().Size()) / kPageSize + 1][(idx - data_.Front().Size()) % kPageSize];
    }
    void Clear() {
        for (size_t i = 0; i < data_.Size(); i++) {
            data_[i].Clear();
        }
        data_.Clear();
        npages_ = 0;
    }

    void PushFront(T elem) {
        if (data_.Size() == 0) {
            // Page<T> tmp;
            data_.PushFront();
            npages_++;
        } else if (data_.Front().Size() == kPageSize) {
            // Page<T> tmp;
            data_.PushFront();
            npages_++;
        }
        data_.Front().PushFront(elem);
    }
    void PushBack(T elem) {
        if (data_.Size() == 0) {
            // fprintf(stderr, "true\n");
            // Page<T> tmp;
            data_.PushBack();
            npages_++;
        } else if (data_.Back().Size() == kPageSize) {
            // fprintf(stderr, "again pushing page\n");
            // Page<T> tmp;
            data_.PushBack();
            npages_++;
        }
        data_.Back().PushBack(elem);
    }
    T PopFront() {
        if (data_.Front().Size() == 0) {
            data_.PopFront();
            npages_--;
        }
        T tmp = data_.Front().PopFront();
        if (data_.Front().Size() == 0) {
            data_.PopFront();
            npages_--;
        }
        return tmp;
    }
    T PopBack() {
        if (data_.Back().Size() == 0) {
            data_.PopBack();
            npages_--;
        }
        T tmp = data_.Back().PopBack();

        if (data_.Back().Size() == 0) {
            data_.PopBack();
            npages_--;
        }
        return tmp;
    }
};

#endif  // DEQUE_DEQUE_H
```


```python

```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.84
    user 0.79
    sys 0.04



```python

```


```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror main.cpp -o a.exe
%run time -p ./a.exe 

#include <vector>


int main() {
    std::vector<int> a;
    for (int i = 0; i < 10'000'000; ++i) {
        a.push_back(i);
    }
    return 0;
}
```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.30
    user 0.24
    sys 0.05



```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror main.cpp -o a.exe
%run time -p ./a.exe 

#include <deque>


int main() {
    std::deque<int> a;
    for (int i = 0; i < 10'000'000; ++i) {
        a.push_back(i);
    }
    return 0;
}
```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.26
    user 0.25
    sys 0.00



```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror main.cpp -o a.exe
%run time -p ./a.exe 

#include "dq.h"


int main() {
    Deque<int> a;
    for (int i = 0; i < 10'000'000; ++i) {
        a.PushBack(i);
    }
    return 0;
}
```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 0.92
    user 0.87
    sys 0.04



```cpp
%%cpp main.cpp
%run clang++ -Wall -Werror main.cpp -o a.exe
%run time -p ./a.exe 

#include "dq.h"


int main() {
    CircularBuffer<int> a;
    for (int i = 0; i < 10'000'000; ++i) {
        a.PushBack();
        a.Back() = i;
    }
    return 0;
}
```


Run: `clang++ -Wall -Werror main.cpp -o a.exe`



Run: `time -p ./a.exe`


    real 3.49
    user 2.58
    sys 0.83



```python

```
