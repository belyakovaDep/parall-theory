#include <iostream>
#include <cmath>

#ifdef USE_FLOAT
using DataType = float;
#else
using DataType = double;
#endif

int main()
{
    DataType* dataArray = new DataType[10000000];
    DataType angle = 0, sum = 0;

    for (int i = 0; i < 10000000; i++)
    {
        angle = (2 * i * 3.14) / 10000000;

        dataArray[i] = static_cast<DataType>(sin(angle));
        sum += dataArray[i];
    }

    std::cout << "Sum is " << sum << std::endl;

    return 0;
}