#include <stdbool.h>

void stack_push_scaler(double);
void stack_push_vec(double, double, double);
bool stack_pop_scaler(double *);
bool stack_pop_vec(double *, double *, double *);
void stack_print_at(int);
void stack_print(void);
bool stack_push(int);
bool stack_swap(int, int);
bool stack_arg_scaler(int);

extern int stack_ptr;
