#include <stdlib.h>

int max_value(int *p_array,unsigned int values_in_array,int *p_max_value)
{
    int position;
    position = 0;
    *p_max_value = p_array[position];

    for (position = 1; position < values_in_array; ++position)
    {
        if (p_array[position] > *p_max_value)
        {
            *p_max_value = p_array[position];
            break;
        }
    }
    return position;
}

unsigned int slice(unsigned int value,unsigned int width,unsigned int offset){
    value = value << (32 - (width + offset));
    value = value >> (32 - width);
    return value;
}

