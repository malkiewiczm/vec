#include <iostream>
#include <string>
#include <cstdlib>
#include <cmath>
#include <utility>
#include <sstream>
#include <cstring>

#ifdef __linux__
#include <unistd.h>
#include <sys/types.h>
#elif _WIN32
#include <windows.h>
#endif

struct Number {
	bool is_scaler;
	double x, y, z;
	Number(): is_scaler(true), x(0.), y(0.), z(0.) {}
	void set(double s) {
		is_scaler = true;
		x = s;
		y = s;
		z = s;
	}
	void set(double l_x, double l_y, double l_z) {
		is_scaler = false;
		x = l_x;
		y = l_y;
		z = l_z;
	}
};

static std::ostream &operator<< (std::ostream &o, const Number &n)
{
	if (n.is_scaler) {
		o << n.x;
	} else {
		o << '(' << n.x << ", " << n.y << ", " << n.z << ')';
	}
	return o;
}

struct Stack {
public:
	static constexpr int SIZE = 200;
	Stack() : ptr(0) {}
	bool empty() const {
		return ptr == 0;
	}
	int size() const {
		return ptr;
	}
	void print() const;
	void push(double x);
	void push(double x, double y, double z);
	void push(const Number &n);
	void clear();
	void dup(int index);
	void swap(int a, int b);
	bool index_ok(int index) const;
	void pop_safe(Number *);
	void pop();
	Number pop_number();
	int pop_int();
	double pop_scaler();
	void copy_top() const;
	void restore();
private:
	Number store[SIZE];
	int ptr;
};

void Stack::print() const
{
	if (empty()) {
		std::cout << "(empty)" << std::endl;
	} else {
		for (int i = std::max(0, ptr - 10); i < ptr; ++i) {
			std::cout << '[' << i << "] " << store[i] << std::endl;
		}
	}
}

void Stack::push(double scaler)
{
	if (ptr == SIZE) {
		return;
	}
	store[ptr].set(scaler);
	++ptr;
}

void Stack::push(double x, double y, double z)
{
	if (ptr == SIZE) {
		return;
	}
	store[ptr].set(x, y, z);
	++ptr;
}

void Stack::push(const Number &n)
{
	if (ptr == SIZE) {
		return;
	}
	store[ptr] = n;
	++ptr;
}

void Stack::clear()
{
	ptr = 0;
}

bool Stack::index_ok(int index) const
{
	if (index < 0) {
		std::cout << "index '" << index << "' out of range (too low)" << std::endl;
		return false;
	}
	if (index >= ptr) {
		std::cout << "index '" << index << "' out of range (too high)" << std::endl;
		return false;
	}
	return true;
}

void Stack::dup(int index)
{
	if (index_ok(index) && ptr != SIZE) {
		store[ptr] = store[index];
		++ptr;
	}
}

void Stack::swap(int a, int b)
{
	if (index_ok(a) && index_ok(b)) {
		std::swap(store[a], store[b]);
	}
}

void Stack::pop_safe(Number *n)
{
	if (ptr == 0) {
		std::cout << "not enough stack space" << std::endl;
	} else {
		--ptr;
		*n = store[ptr];
	}
}

void Stack::pop()
{
	Number dummy;
	pop_safe(&dummy);
}

Number Stack::pop_number()
{
	Number n;
	pop_safe(&n);
	return n;
}

int Stack::pop_int()
{
	return static_cast<int>(pop_scaler());
}

double Stack::pop_scaler()
{
	Number n;
	pop_safe(&n);
	return n.x;
}

void Stack::restore()
{
	if (ptr != SIZE) {
		++ptr;
	}
}

void Stack::copy_top() const
{
	if (empty()) {
		std::cout << "stack empty, nothing to copy" << std::endl;
		return;
	}
	std::stringstream ss;
	ss.precision(15);
	ss << store[ptr - 1];
	const std::string clipboard_text = ss.str();
#ifdef __linux__
	int fd[2];
	if (pipe(fd)) {
		std::cout << "Clipboard failed (pipe)" << std::endl;
		return;
	}
	pid_t pid = fork();
	if (pid == -1) {
		std::cout << "clipboard failed (fork)" << std::endl;
	} else if (pid == 0) {
		// child closes write pipe
		close(fd[1]);
		dup2(fd[0], 0);
		close(fd[0]);
		execl("/bin/sh", "sh", "-c", "xclip -selection clipboard", NULL);
		std::cout << "clipboard failed (in child process, execve failed)" << std::endl;
		exit(0);
	} else {
		// parent closes read pipe
		close(fd[0]);
		if (write(fd[1], clipboard_text.c_str(), clipboard_text.size()) != clipboard_text.size())
			std::cout << "clipboard failed (could not write to pipe)" << std::endl;
		close(fd[1]);
	}
#elif _WIN32
	HGLOBAL clipboard_handle = GlobalAlloc(GMEM_MOVEABLE, clipboard_text.size() + 1);
	std::memcpy(GlobalLock(clipboard_handle), clipboard_text.c_str(), clipboard_text.size() + 1);
	GlobalUnlock(clipboard_handle);
	if (! OpenClipboard(NULL)) {
		std::cout << "clipboard failed (OpenClipboard)" << std::endl;
		return;
	}
	EmptyClipboard();
	if (SetClipboardData(CF_TEXT, clipboard_handle) == NULL)
		std::cout << "clipboard failed (SetClipboardData)" << std::endl;
	CloseClipboard();
#else
	std::cout << "clipboard not supported" << std::endl;
#endif
}

static bool as_double(const std::string &s, double *value)
{
	try {
		*value = std::stod(s);
		return true;
	} catch(...) {
		return false;
	}
}

static Stack stack;

#define cmd(a) else if(s == (a))

#define mimic_prefix1(what) do {				\
		Number n = stack.pop_number();			\
		n.x = what(n.x);						\
		n.y = what(n.y);						\
		n.z = what(n.z);						\
		stack.push(n);							\
	} while (0);

#define mimic_prefix2(what) do {						\
		const Number rhs = stack.pop_number();			\
		const Number lhs = stack.pop_number();			\
		Number n;										\
		n.is_scaler = lhs.is_scaler && rhs.is_scaler;	\
		n.x = what(lhs.x, rhs.x);						\
		n.y = what(lhs.y, rhs.y);						\
		n.z = what(lhs.z, rhs.z);						\
		stack.push(n);									\
	} while (0);

#define mimic_infix(what) do {							\
		const Number rhs = stack.pop_number();			\
		const Number lhs = stack.pop_number();			\
		Number n;										\
		n.is_scaler = lhs.is_scaler && rhs.is_scaler;	\
		n.x = lhs.x what rhs.x;							\
		n.y = lhs.y what rhs.y;							\
		n.z = lhs.z what rhs.z;							\
		stack.push(n);									\
	} while (0);

static void parse(const std::string &s)
{
	double value;
	if (as_double(s, &value)) {
		stack.push(value);
	}
	cmd("exit") {
		std::exit(0);
	}
	cmd("cls" ) {
		stack.clear();
	}
	cmd("pop") {
		stack.pop();
	}
	cmd("restore") {
		stack.restore();
	}
	cmd("push") {
		stack.dup(stack.pop_int());
	}
	cmd("dup") {
		stack.dup(stack.size() - 1);
	}
	cmd("swap") {
		const int a = stack.pop_int();
		stack.swap(a, stack.size() - 1);
	}
	cmd("cp") {
		stack.copy_top();
	}
	cmd("vec") {
		const double z = stack.pop_scaler();
		const double y = stack.pop_scaler();
		const double x = stack.pop_scaler();
		stack.push(x, y, z);
	}
	cmd("dot") {
		const Number rhs = stack.pop_number();
		const Number lhs = stack.pop_number();
		stack.push(lhs.x*rhs.x + lhs.y*rhs.y + lhs.z*rhs.z);
	}
	cmd("cross") {
		const Number rhs = stack.pop_number();
		const Number lhs = stack.pop_number();
		stack.push(lhs.y*rhs.z - lhs.z*rhs.y, lhs.z*rhs.x - lhs.x*rhs.z, lhs.x*rhs.y - lhs.y*rhs.x);
	}
	cmd("magsq") {
		const Number n = stack.pop_number();
		stack.push(n.x*n.x + n.y*n.y + n.z*n.z);
	}
	cmd("mag") {
		const Number n = stack.pop_number();
		stack.push(std::sqrt(n.x*n.x + n.y*n.y + n.z*n.z));
	}
	cmd("*") {
		mimic_infix(*);
	}
	cmd("/") {
		mimic_infix(/);
	}
	cmd("%") {
		mimic_prefix2(std::fmod);
	}
	cmd("+") {
		mimic_infix(+);
	}
	cmd("-") {
		mimic_prefix1(-);
	}
	cmd("++") {
		Number res;
		while (! stack.empty()) {
			const Number n = stack.pop_number();
			res.x += n.x;
			res.y += n.y;
			res.z += n.z;
			if (! n.is_scaler)
				res.is_scaler = false;
		}
		stack.push(res);
	}
	cmd("**") {
		Number res;
		res.set(1.);
		while (! stack.empty()) {
			const Number n = stack.pop_number();
			res.x *= n.x;
			res.y *= n.y;
			res.z *= n.z;
			if (! n.is_scaler)
				res.is_scaler = false;
		}
		stack.push(res);
	}
	cmd("inv") {
		mimic_prefix1(1. / );
	}
	cmd("pow") {
		mimic_prefix2(std::pow);
	}
	cmd("polar") {
		const double angle = stack.pop_scaler();
		stack.push(std::cos(angle), std::sin(angle), 0);
	}
	cmd("cos") {
		mimic_prefix1(std::cos);
	}
	cmd("sin") {
		mimic_prefix1(std::sin);
	}
	cmd("tan") {
		mimic_prefix1(std::tan);
	}
	cmd("asin") {
		mimic_prefix1(std::asin);
	}
	cmd("acos") {
		mimic_prefix1(std::acos);
	}
	cmd("atan") {
		mimic_prefix1(std::atan);
	}
	cmd("sqrt") {
		mimic_prefix1(std::sqrt);
	}
	cmd("log") {
		mimic_prefix1(std::log);
	}
	cmd("log10") {
		mimic_prefix1(std::log10);
	}
	cmd("log2") {
		mimic_prefix1(std::log2);
	}
	cmd("torad") {
		mimic_prefix1(0.017453292519943 * );
	}
	cmd("todeg") {
		mimic_prefix1(57.295779513082 * );
	}
	cmd("atan2") {
		mimic_prefix2(std::atan2);
	}
	cmd("floor") {
		mimic_prefix1(std::floor);
	}
	cmd("ceil") {
		mimic_prefix1(std::ceil);
	}
	cmd("round") {
		mimic_prefix1(std::round);
	}
	cmd("x") {
		stack.push(stack.pop_number().x);
	}
	cmd("y") {
		stack.push(stack.pop_number().y);
	}
	cmd("z") {
		stack.push(stack.pop_number().z);
	}
	cmd("unvec") {
		const Number n = stack.pop_number();
		stack.push(n.x);
		stack.push(n.y);
		stack.push(n.z);
	}
	cmd("tovec") {
		Number n = stack.pop_number();
		n.is_scaler = false;
		stack.push(n);
	}
	cmd("pi") {
		stack.push(3.1415926535898);
	}
	cmd("e") {
		stack.push(2.71828);
	}
	cmd("ee") {
		stack.push(1.60217733e-19);
	}
	cmd("me") {
		stack.push(9.10938356e-31);
	}
	cmd("mp") {
		stack.push(1.672621898e-27);
	}
	cmd("ivec") {
		stack.push(1, 0, 0);
	}
	cmd("jvec") {
		stack.push(0, 1, 0);
	}
	cmd("kvec") {
		stack.push(0, 0, 1);
	}
	cmd("quadeq") {
		const Number eq = stack.pop_number();
		const double disc = eq.y*eq.y - 4.*eq.x*eq.z;
		const double left = -eq.y / (2.*eq.x);
		const double right = std::sqrt(disc) / (2.*eq.x);
		stack.push(left - right, left + right, 0.);
	}
	else {
		std::cout << "command not found" << std::endl;
	}
}

int main()
{
	std::cout.precision(15);
	for ( ; ; ) {
		stack.print();
		std::string line;
		std::cout << "> ";
		if (! std::getline(std::cin, line))
			break;
		std::stringstream ss(line);
		std::string token;
		while (ss >> token) {
			parse(token);
		}
	}
	std::cout << std::endl;
}
