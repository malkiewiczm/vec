#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "stack.h"

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#elif _WIN32
#include <windows.h>
#endif

#define ARG_SS 3
#define ARG_VS 1
#define ARG_SV 2
#define ARG_VV 0

#define cmd(a)(strcmp(str, a) == 0)

#define ss(n) do {								\
		if (stack_ptr < n) {					\
			puts("Not enough stack space");		\
			return;								\
		}										\
	} while(0);

#define args()(((stack_arg_scaler(1) << 1) | stack_arg_scaler(0)) & 3)
#define mimick(what) do {						\
		ss(1);									\
		if (stack_pop_scaler(&value))			\
			stack_push_scaler(what(value));		\
	} while(0);

static inline void parse(char *str)
{
	double value;
	if (sscanf(str, "%lf", &value) == 1) {
		stack_push_scaler(value);
		return;
	}
	if (cmd("exit")) {
		exit(0);
	} else if (cmd("cls")) {
		stack_ptr = 0;
	} else if (cmd("pop")) {
		ss(1);
		stack_ptr--;
	} else if (cmd("restore")) {
		// this could give some garbage values
		stack_ptr++;
		if (stack_ptr == STACK_SIZE) {
			stack_ptr--;
			puts("Stack overflow");
		}
	} else if (cmd("push")) {
		ss(1);
		double n;
		if (stack_pop_scaler(&n))
			stack_push((int)n);
	} else if (cmd("dup")) {
		ss(1);
		stack_push(stack_ptr);
	} else if (cmd("swap")) {
		ss(1);
		double n;
		if (stack_pop_scaler(&n))
			stack_swap((int)n, stack_ptr);
	} else if (cmd("vec")) {
		ss(3);
		double x, y, z;
		if (stack_pop_scaler(&z) && stack_pop_scaler(&y) && stack_pop_scaler(&x))
			stack_push_vec(x, y, z);
	} else if (cmd("dot")) {
		ss(2);
		double ax, ay, az;
		double bx, by, bz;
		if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
			stack_push_scaler(ax * bx + ay * by + az * bz);
	} else if (cmd("cross")) {
		ss(2);
		double ax, ay, az;
		double bx, by, bz;
		if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
			stack_push_vec(ay * bz - az * by, az * bx - ax * bz, ax * by - ay * bx);
	} else if (cmd("magsq")) {
		ss(1);
		double ax, ay, az;;
		if (stack_pop_vec(&ax, &ay, &az))
			stack_push_scaler(ax * ax + ay * ay + az * az);
	} else if (cmd("mag")) {
		ss(1);
		double ax, ay, az;;
		if (stack_pop_vec(&ax, &ay, &az))
			stack_push_scaler(sqrt(ax * ax + ay * ay + az * az));
	} else if (cmd("*")) {
		ss(2);
		switch (args()) {
		case ARG_VV: {
			// this is neither the dot nor cross product
			double ax, ay, az;
			double bx, by, bz;
			if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
				stack_push_vec(ax * bx, ay * by, az * bz);
			break;
		}
		case ARG_SS: {
			double a, b;
			if (stack_pop_scaler(&b) && stack_pop_scaler(&a))
				stack_push_scaler(a * b);
			break;
		}
		case ARG_VS: {
			double x, y, z, s;
			if (stack_pop_scaler(&s) && stack_pop_vec(&x, &y, &z))
				stack_push_vec(x * s, y * s, z * s);
			break;
		}
		case ARG_SV: {
			double x, y, z, s;
			if (stack_pop_vec(&x, &y, &z) && stack_pop_scaler(&s))
				stack_push_vec(x * s, y * s, z * s);
			break;
		}
		}
	} else if (cmd("/")) {
		ss(2);
		switch (args()) {
		case ARG_VV: {
			// probably not the behavior the user is expecting
			double ax, ay, az;
			double bx, by, bz;
			if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
				stack_push_vec(ax / bx, ay / by, az / bz);
			break;
		}
		case ARG_SS: {
			double a, b;
			if (stack_pop_scaler(&b) && stack_pop_scaler(&a))
				stack_push_scaler(a / b);
			break;
		}
		case ARG_VS: {
			double x, y, z, s;
			if (stack_pop_scaler(&s) && stack_pop_vec(&x, &y, &z))
				stack_push_vec(x / s, y / s, z / s);
			break;
		}
		case ARG_SV: {
			double x, y, z, s;
			if (stack_pop_vec(&x, &y, &z) && stack_pop_scaler(&s))
				stack_push_vec(s / x, s / y, s / z);
			break;
		}
		}
	} else if (cmd("+")) {
		ss(2);
		switch (args()) {
		case ARG_VV: {
			double ax, ay, az, bx, by, bz;
			if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
				stack_push_vec(ax + bx, ay + by, az + bz);
			break;
		}
		case ARG_SS: {
			double a, b;
			if (stack_pop_scaler(&b) && stack_pop_scaler(&a))
				stack_push_scaler(a + b);
			break;
		}
		case ARG_VS: {
			double x, y, z, s;
			if (stack_pop_scaler(&s) && stack_pop_vec(&x, &y, &z))
				stack_push_vec(x + s, y + s, z + s);
			break;
		}
		case ARG_SV: {
			double x, y, z, s;
			if (stack_pop_vec(&x, &y, &z) && stack_pop_scaler(&s))
				stack_push_vec(s + x, s + y, s + z);
			break;
		}
		}
	} else if (cmd("%")) {
		ss(2);
		switch (args()) {
		case ARG_VV: {
			double ax, ay, az, bx, by, bz;
			if (stack_pop_vec(&bx, &by, &bz) && stack_pop_vec(&ax, &ay, &az))
				stack_push_vec(fmod(ax,  bx), fmod(ay, by), fmod(az, bz));
			break;
		}
		case ARG_SS: {
			double a, b;
			if (stack_pop_scaler(&b) && stack_pop_scaler(&a))
				stack_push_scaler(fmod(a, b));
			break;
		}
		case ARG_VS: {
			double x, y, z, s;
			if (stack_pop_scaler(&s) && stack_pop_vec(&x, &y, &z))
				stack_push_vec(fmod(x, s), fmod(y, s), fmod(z, s));
			break;
		}
		case ARG_SV: {
			double x, y, z, s;
			if (stack_pop_vec(&x, &y, &z) && stack_pop_scaler(&s))
				stack_push_vec(fmod(s, x), fmod(s, y), fmod(s, z));
			break;
		}
		}
	} else if (cmd("++")) {
		ss(1);
		double sum = 0;
		double s;
		while (stack_pop_scaler(&s))
			sum += s;
		stack_push_scaler(sum);

	} else if (cmd("-")) {
		ss(1);
		if (stack_arg_scaler(0)) {
			if (stack_pop_scaler(&value))
				stack_push_scaler(- value);
		} else {
			double x, y, z;
			if (stack_pop_vec(&x, &y, &z))
				stack_push_vec(-x, -y, -z);
		}
	} else if (cmd("inv")) {
		ss(1);
		if (stack_arg_scaler(0)) {
			if (stack_pop_scaler(&value))
				stack_push_scaler(1 / value);
		} else {
			double x, y, z;
			if (stack_pop_vec(&x, &y, &z))
				stack_push_vec(1 / x, 1 / y, 1 / z);
		}
	} else if (cmd("pow")) {
		ss(2);
		double a, b;
		if (stack_pop_scaler(&b) && stack_pop_scaler(&a)) {
			stack_push_scaler(pow(a, b));
		}
	} else if (cmd("polar")) {
		ss(1);
		double a;
		if (stack_pop_scaler(&a)) {
			stack_push_vec(cos(a), sin(a), 0);
		}
	} else if (cmd("quadeq")) {
		ss(1);
		double a, b, c;
		if (stack_pop_vec(&a, &b, &c)) {
			double det = b * b - 4 * a * c;
			if (det < 0) {
				puts("No real solutions");
				return;
			}
			double den = 2 * a;
			double x0 = -b / den;
			double x1 = sqrt(det) / den;
			stack_push_vec(x0 + x1, x0 - x1, 0);
		}
	} else if (cmd("cp")) {
		if (! stack_ptr) {
			puts("Stack empty, nothing to copy");
			return;
		}
		static char *clipboard_text = NULL;
		static const size_t clipboard_size = sizeof(char) * 100;
		if (clipboard_text == NULL)
			clipboard_text = malloc(clipboard_size);
		stack_print_str_at(clipboard_text, stack_ptr);
#ifdef __linux__
		int fd[2];
		if (pipe(fd)) {
			puts("Clipboard failed (pipe)");
			return;
		}
		pid_t pid = fork();
		if (pid == -1) {
			puts("Clipboard failed (fork)");
		} else if (pid == 0) {
			// child closes write pipe
			close(fd[1]);
			dup2(fd[0], 0);
			close(fd[0]);
			execl("/bin/sh", "sh", "-c", "xclip -selection clipboard", NULL);
			puts("Clipboard failed (in child process, execve failed)");
			exit(0);
		} else {
			// parent closes read pipe
			close(fd[0]);
			int text_len = strlen(clipboard_text);
			if (write(fd[1], clipboard_text, text_len) != text_len)
				puts("Clipboard failed (could not write to pipe)");
			close(fd[1]);
		}
#elif _WIN32
		HGLOBAL clipboard_handle = GlobalAlloc(GMEM_MOVEABLE, clipboard_size);
		memcpy(GlobalLock(clipboard_handle), clipboard_text, clipboard_size);
		GlobalUnlock(clipboard_handle);
		if (! OpenClipboard(NULL)) {
			puts("Clipboard failed (OpenClipboard)");
			return;
		}
		EmptyClipboard();
		if (SetClipboardData(CF_TEXT, clipboard_handle) == NULL)
			puts("Clipboard failed (SetClipboardData)");
		CloseClipboard();
#else
		puts("Clipboard not supported");
#endif
	} else if (cmd("cos")) {
		mimick(cos);
	} else if (cmd("sin")) {
		mimick(sin);
	} else if (cmd("tan")) {
		mimick(tan);
	} else if (cmd("asin")) {
		mimick(asin);
	} else if (cmd("acos")) {
		mimick(acos);
	} else if (cmd("atan")) {
		mimick(atan);
	} else if (cmd("sqrt")) {
		mimick(sqrt);
	} else if (cmd("log")) {
		mimick(log);
	} else if (cmd("log10")) {
		mimick(log10);
	} else if (cmd("log2")) {
		mimick(log2);
	} else if (cmd("torad")) {
		mimick(0.017453292519943 * );
	} else if (cmd("todeg")) {
		mimick(57.295779513082 * );
	} else if (cmd("atan2")) {
		ss(2);
		double a, b;
		if (stack_pop_scaler(&b) && stack_pop_scaler(&a))
			stack_push_scaler(atan2(a, b));
	} else if (cmd("int")) {
		ss(1);
		if (stack_arg_scaler(0)) {
			stack_pop_scaler(&value);
			stack_push_scaler((int)value);
		} else {
			double ax, ay, az;
			stack_pop_vec(&ax, &ay, &az);
			stack_push_vec((int)ax, (int)ay, (int)az);
		}
	} else if (cmd("x")) {
		ss(1);
		double x, y, z;
		if (stack_pop_vec(&x, &y, &z))
			stack_push_scaler(x);
	} else if (cmd("y")) {
		ss(1);
		double x, y, z;
		if (stack_pop_vec(&x, &y, &z))
			stack_push_scaler(y);
	} else if (cmd("z")) {
		ss(1);
		double x, y, z;
		if (stack_pop_vec(&x, &y, &z))
			stack_push_scaler(z);
	} else if (cmd("unvec")) {
		ss(1);
		double x, y, z;
		if (stack_pop_vec(&x, &y, &z)) {
			stack_push_scaler(x);			
			stack_push_scaler(y);
			stack_push_scaler(z);
		}
	} else if (cmd("pi")) {
		stack_push_scaler(3.1415926535898);
	} else if (cmd("e")) {
		stack_push_scaler(2.71828);
	} else if (cmd("ee")) {
		stack_push_scaler(1.60217733e-19);
	} else if (cmd("me")) {
		stack_push_scaler(9.10938356e-31);
	} else if (cmd("mp")) {
		stack_push_scaler(1.672621898e-27);
	} else if (cmd("ivec")) {
		stack_push_vec(1, 0, 0);
	} else if (cmd("jvec")) {
		stack_push_vec(0, 1, 0);
	} else if (cmd("kvec")) {
		stack_push_vec(0, 0, 1);
	} else {
		puts("Command not found");
	}
}

int main(int argc, char **argv)
{
	if (argc > 1) {
		for (int iter = 1; iter < argc; ++iter) {
			parse(argv[iter]);
		}
		stack_print();
		return 0;
	}
	while (true) {
		char buf[100];
		stack_print();
		printf("> ");
		if (fgets(buf, 100, stdin) == NULL) {
			puts("EOF detected");
			return 0;
		}
		if (buf[0] == '\0') {
			putchar('\n');
			return 0;
		}
		int start = 0;
		for (int i = 0; i < 100; i++) {
			bool terminate = buf[i] == '\n';
			if (buf[i] <= ' ') {
				buf[i] = '\0';
				if (i - start)
					parse(buf + start);
				start = i + 1;
			}
			if (terminate)
				break;
		}
	}
}
