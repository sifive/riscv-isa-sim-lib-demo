// PERIA occupies all custom 0 page

#define MATCH_PERI_A_ADD 0x0000000b
// R type ADD 
#define MASK_PERI_A_ADD  0xfe00707f 

// peri v will occupy all custom1 page
#define MATCH_PERI_V_ADD 0x0000002b
// vector-like instruction 
// funct6 
#define MASK_PERI_V_ADD  0xfc00707f