#ifndef PROCESSOR_H
#define PROCESSOR_H

class Processor {
 public:
  float Utilization();
  void Utilization(float util);

  private:
    float util_;
};

#endif