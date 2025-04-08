// MyClass.h
#ifndef MYCLASS_H
#define MYCLASS_H

#include "ref_obj.h"
#include <string>
#include <iostream>

class NestedClass : public Reflectable {
public:
	NestedClass() {
		REFLECTABLE_FIELD(nestedIntField)
	}

	int nestedIntField = 100;
};

class MyClass : public Reflectable {
public:
	MyClass() {
		REFLECTABLE_FIELD(intField)
		REFLECTABLE_FIELD(doubleField)
		REFLECTABLE_NESTED_FIELD(nestedObj)
	}

	int intField = 42;
	double doubleField = 3.14;
	NestedClass nestedObj;
};

#endif // MYCLASS_H