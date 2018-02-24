#include <stdio.h>
#include "stack.h"

struct StackVal {
	bool is_scaler;
	double x;
	double y;
	double z;
};

static struct StackVal stack[STACK_SIZE];
int stack_ptr = 0;

void stack_push_scaler(double value)
{
	stack_ptr++;
	if (stack_ptr == STACK_SIZE) {
		puts("Stack overflow");
		stack_ptr--;
		return;
	}
	stack[stack_ptr].is_scaler = true;
	stack[stack_ptr].x = value;
}

void stack_push_vec(double x, double y, double z)
{
	stack_ptr++;
	if (stack_ptr == STACK_SIZE) {
		puts("Stack overflow");
		stack_ptr--;
		return;
	}
	stack[stack_ptr].is_scaler = false;
	stack[stack_ptr].x = x;
	stack[stack_ptr].y = y;
	stack[stack_ptr].z = z;
}

bool stack_pop_scaler(double *value)
{
	if (! stack_ptr) {
		puts("Stack underflow");
		return false;
	}
	if (! stack[stack_ptr].is_scaler) {
		puts("Expected scaler, got vector");
		return false;
	}
	*value = stack[stack_ptr].x;
	stack_ptr--;
	return true;
}

bool stack_pop_vec(double *x, double *y, double *z)
{
	if (! stack_ptr) {
		puts("Stack underflow");
		return false;
	}
	if (stack[stack_ptr].is_scaler) {
		puts("Expected vector, got scaler");
		return false;
	}
	*x = stack[stack_ptr].x;
	*y = stack[stack_ptr].y;
	*z = stack[stack_ptr].z;
	stack_ptr--;
	return true;
}

void stack_print_at(int n)
{
	if (stack[n].is_scaler) {
		printf("%.15lf\n", stack[n].x);
	} else {
		printf("(%lf, %lf, %lf)\n", stack[n].x, stack[n].y, stack[n].z);
	}
}

#define max(a, b)((a > b) ? a : b)

void stack_print()
{
	if (stack_ptr) {
		for (int i = max(1, stack_ptr - 10); i <= stack_ptr; i++) {
			printf("[%d] ", i);
			stack_print_at(i);
		}
	} else {
		puts("(empty)");
	}
}

bool stack_push(int n)
{
	if (n <= 0) {
		puts("Out of range (too low)");
		return false;
	}
	if(n > stack_ptr) {
		puts("Out of range (too high)");
		return false;
	}
	stack_ptr++;
	stack[stack_ptr] = stack[n];
	return true;
}

bool stack_swap(int a, int b)
{
	if (a <= 0 || b <= 0) {
		puts("Out of range (too low)");
		return false;
	}
	if(a > stack_ptr || b > stack_ptr) {
		puts("Out of range (too high)");
		return false;
	}
	struct StackVal tmp = stack[a];
	stack[a] = stack[b];
	stack[b] = tmp;
	return true;
}

bool stack_arg_scaler(int n)
{
	return stack[stack_ptr - n].is_scaler;
}
