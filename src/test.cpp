#include <iostream>

using namespace std;

typedef int (*OperationFunction)(int, int);

int add(int a, int b) {
	return a + b;
}

int mult(int a, int b) {
	return a * b;
}

int transform(int a, int b, OperationFunction fn) {
	return fn(a, b);
}



int main(void) {

	OperationFunction addFn = &add;
	OperationFunction multFn = &mult;

	cout << transform(3, 5, addFn) << endl;
	cout << transform(3, 5, multFn) << endl;


	return 0;
}