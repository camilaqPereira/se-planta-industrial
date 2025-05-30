#ifndef UTILS_H
#define UTILS_H


/* Macros definition*/
#define set_bit(port, bit_n) (port|=1<<bit_n)       // Set bit n in port
#define clr_bit(port, bit_n) (port&=~(1<<bit_n))    // Clear bit n in port
#define toggle_bit(port, bit_n) (port^=(1<<bit_n))  // Toggle bit n in port
#define test_bit(port, bit_n) (port&(1<<bit_n))     // Test bit n in port


#endif