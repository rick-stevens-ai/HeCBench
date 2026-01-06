#ifndef _REFERENCE_H_
#define _REFERENCE_H_

template <typename T>
void computeGold(T *gpuData, unsigned int len)
{
  T val0 = (T)(len * 10);
  T val1 = 0 - (T)(len * 10);
  T val2 = (T)(len - 1);
  T val3 = 0;
  T val4 = 255;
  T val5 = (1 << (len - 1)) - 1;
  T val6 = (T)((len - 1) * len / 2);
  //T val7 = 17;
  //T val8 = 0;

  if (gpuData[0] != val0)
    printf("atomicAdd failed val0=%d data0=%d\n", (int)val0, (int)gpuData[0]);
  if (gpuData[1] != val1)
    printf("atomicSub failed val1=%d data1=%d\n", (int)val1, (int)gpuData[1]);
  if (gpuData[2] != val2)
    printf("atomicMax failed val2=%d data2=%d\n", (int)val2, (int)gpuData[2]);
  if (gpuData[3] != val3)
    printf("atomicMin failed val3=%d data3=%d\n", (int)val3, (int)gpuData[3]);
  if (gpuData[4] != val4)
    printf("atomicAnd failed val4=%d data4=%d\n", (int)val4, (int)gpuData[4]);
  if (gpuData[5] != val5)
    printf("atomicOr failed val5=%d data5=%d\n", (int)val5, (int)gpuData[5]);
  if (gpuData[6] != val6)
    printf("atomicXor failed val6=%d data6=%d\n", (int)val6, (int)gpuData[6]);
  /*
  if (gpuData[7] != val7)
    printf("atomicInc failed val7=%d data7=%d\n", (int)val7, (int)gpuData[7]);
  if (gpuData[8] != val8)
    printf("atomicDec failed val8=%d data8=%d\n", (int)val8, (int)gpuData[8]);
  */
}

#endif
