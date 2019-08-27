
#ifndef WORKLOAD_H_
#define WORKLOAD_H_

#include <vector>

class WorkLoad {
 public:
  virtual ~WorkLoad() { }
  virtual size_t read(size_t size) = 0;
  virtual size_t write(size_t size) = 0;
};


class BasicWorkLoad : public WorkLoad {
 public:
  virtual size_t read(size_t size) {
    volatile int dummy = size;
    for (size_t i = 0; i < size; i++) {
      dummy = dummy + dummy;
    }
    return dummy;
  }

  virtual size_t write(size_t size) {
    volatile int dummy = size;
    for (size_t i = 0; i < size; i++) {
      dummy = dummy + dummy;
    }
    return dummy;
  }
};


class VectorWorkLoad : public WorkLoad {
  
 private:
  std::vector<int> data;

 public:
 VectorWorkLoad(int size) : data() {
    for (int i = 0; i < size; i++) {
      data.push_back(i);
    }
  }

  virtual size_t read(size_t size) {
    int dummy = 0;
    int n = data.size();
    for (size_t i = 0; i < size; i++) {
      dummy += data[i % n];
    }
    return dummy;
  }

  virtual size_t write(size_t size) {
    int dummy = 0;
    int n = data.size();
    for (size_t i = 0; i < size; i++) {
      data[i % n] = i;
    }
    return dummy;
  }
};

#endif
