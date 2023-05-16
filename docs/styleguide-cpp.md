# OpenVic2 C++ Style Guidelines

## Table of Contents
 1. [Why Style?](styleguide-cpp.md#1-why-style)

	1.1. [General Principles](styleguide-cpp.md#11-general-principles)

	1.2 [File Formatting](styleguide-cpp.md#12-file-formatting)

 2. [Conventions](styleguide-cpp.md#2-conventions)

	2.1. [Naming Conventions](styleguide-cpp.md#21-naming-conventions)

	2.2 [Do's and Don'ts](styleguide-cpp.md#22-dos-and-donts)

## 1. Why Style?
>“Programs are meant to be read by humans and only incidentally for computers to execute.”
> - H. Abelson and G. Sussman in [*Structure and Interpretation of Computer Programs*](https://en.wikipedia.org/wiki/Structure_and_Interpretation_of_Computer_Programs)

The purpose of this styleguide is to give a clear framework for other contributors, to minimize ambiguity, and to have a consistant way to convey the intent of code.

### 1.1. General Principles
- Prefer clarity over brevity
- [Don’t optimize prematurely](https://youtu.be/tKbV6BpH-C8)
- In cases of ambiguity, use your best judgement

### 1.2. File Formatting
Source code files should adhere to the following:

- Encoded in UTF-8
- Use tabs for indentation
- Use LF for end-of-line sequences
- No trailing whitespace (Lines which end in spaces or tabs)
- Any `#include` directives should be at the top of the file
- Any header files have the `.hpp` file extension
- Any implementation files have the `.cpp` file extension
- Code should be within the `OpenVic` namespace (or a nested namespace)
- Any submodules or third-party utilities should be in the `OpenVic/extension/deps` directory
- Groups of related source files should be kept in a sub-directory of `OpenVic/extension/src`
- Any lines that are longer than 120 characters in length should be split across multiple lines

## 2. Conventions
### 2.1 Naming Conventions
| Item | Writing Convention | Example |
|--|--|--|
| Class names | Pascal Case | MyCoolExample |
| Struct names | Pascal Case | MyCoolExample |
| Variable and Attribute names | Camel Case | myCoolExample |
| Function and Method names | Camel Case | myCoolExample |
| Constants | Macro Case | MY_COOL_EXAMPLE |
| Macros | Macro Case | MY_COOL_EXAMPLE |
| Class or container type aliases | Pascal Case | MyCoolExample |
| Builtin or numeric type aliases | Camel Case ( `_t` suffix) | myCoolExample_t |
| Enum names | Pascal Case | MyCoolExample |
| Enum values | Pascal Case | MyCoolExample |


### 2.2 Do's and Don'ts
- Keep the opening scope brace on the same line
	```c++
	//Correct
	if (condition) {
		//Cool things here
	}

	//Incorrect
	if (condition)
	{
		//cool things here
	}
	```
- With arithmetic, relational, and logical operators keep spaces between variables and literals
	```c++
	//Correct
	int myNumber = (someVariable + 10) % 3;

	//Incorrect
	int myNumber=(someVariable+10)%3;
	```
- Prefix and postfix operators should be immediately attached to the affected variable
	```c++
	//Correct
	for (int x = 0; x < 10; x++) {
		//Something cool
	}

	//Incorrect
	for (int x = 0; x < 10; x ++ ) {
		//Something cool
	}
	```
- The content of classes should appear in the following order:
	- Type Aliases
	- Static (class-wide) atributes
	- Attributes
	- Constructors and Destructors
	- Static (class-wide) methods
	- Methods

	```c++
	class Human {
		public:
		using name_t = std::string;

		private:
		static size_t numberOfHumans;

		public:
		name_t name;
		size_t age;
		size_t familyMembers;

		Human();
		~Human();

		static bool isMoreHumansThan(size_t number);


		private:
		void coolPrivateMethod();

		public:
		void coolPublicMethod();
		void anotherCoolPublicMethod();

		inline bool isVotingAge() const {
			return age >= 18;
		}
	};
	```
- Class visibility specifiers should be followed by the next declaration on a new line at the same indentation level
	- Group methods and attributes by their visibility specifier from most to least restrictive (`private`, `protected`, then `public`)
	```c++
	class ExampleClass {
		//Attributes...

		private:
		void firstPrivateMethod();
		void secondPrivateMethod();
		void thirdPrivateMethod();

		protected:
		void firstProtectedMethod();
		void secondProtectedMethod();
		void thirdProtectedMethod();

		public:
		void firstPublicMethod();
		void secondPublicMethod();
		void thirdPublicMethod();
	};
	```
- Methods which do not mutate an object's state should be marked as a `const` method
	```c++
	//Correct
	bool RGBColour::isColourBlack() const {
		return redChannel == 0 && greenChannel == 0 && blueChannel == 0;
	}

	//Incorrect
	bool RGBColour::isColourBlack() {
		return redChannel == 0 && greenChannel == 0 && blueChannel == 0;
	}
	```
- When argument or parameter lists are too long to easily fit on one line: begin the argument list on the next line, two indentation levels higher than the function's indentation.
	```c++
	//Correct
	callingAFunctionWithAVeryLongNameAndManyArguments(
			longArgumentNameOne, longArgumentNameTwo, longArgumentNameThree,
			longArgumentNameFour, longArgumentNameFive, longArgumentNameSix,
			longArgumentNameSeven, longArgumentNameEight);

	//Incorrect
	callingAFunctionWithAVeryLongNameAndManyArguments(longArgumentNameOne, longArgumentNameTwo, longArgumentNameThree, longArgumentNameFour, longArgumentNameFive, longArgumentNameSix, longArgumentNameSeven, longArgumentNameEight);
	```
- In header files, put `#pragma once` at the top of the file, not C-style header guards
	```c++
	//Correct
	#pragma once
	/*
	header contents here
	*/

	//Incorrect
	#ifndef MY_COOL_HEADER_H
	#define MY_COOL_HEADER_H
	/*
	header contents here
	*/
	#endif
	```
- Use enum classes, not plain enums
	```c++
	//Correct
	enum class Colours { Red, Green, Blue, CharcoalGrey };

	//Incorrect
	enum Colours { Red, Green, Blue, CharcoalGrey };
	```
- Use type aliases, not `typedef`s
	```c++
	//Correct
	using MyContainer = std::vector<int>;
	using byte_t = unsigned char;

	//Incorrect
	typedef std::vector<int> MyContainer;
	typedef unsigned char byte_t;
	```
- Use explicit types, do not use `auto`
	```c++
	//Correct
	std::pair<int, int> coordinate = getPlayerLocation();

	//Incorrect
	auto coordinate = getPlayerLocation();
	```
- Do not use C-style casts, instead use C++ casts (ie: `static_cast`, `const_cast`, `reinterpret_cast`)
	- `dynamic_cast` should only be used with caution
	```c++
	//Correct
	int myNumber = static_cast<int>(someFloatValue);

	//Incorrect
	int myNumber = (int) someFloatValue;
	```
- When declaring a pointer, put the asterisk immediately after the type, followed by a space and the variable name
	```c++
	//Correct
	int* myPointer = &myNumber;

	//Incorrect
	int * myPointer = &myNumber;
	int *myPointer = &myNumber;
	int*myPointer = &myNumber;
	```
- Do not declare multiple pointer variables on a single line
	```c++
	//Correct
	int* p1;
	int* p2;

	//Incorrect
	int *p1, *p2;
	```
- Keep the initialization of a variable and its declaration as close as possible, ideally on the same line.
	```c++
	//Correct
	int myNumber = 23;

	//Incorrect
	int myNumber;
	myNumber = 23;
	```
- For constant values that are computable at compile-time, use `constexpr` instead of the `const` keyword
	```c++
	//Correct
	constexpr size_t UNIQUE_RGB_COLOURS = 256 * 256 * 256;

	//Incorrect
	const size_t UNIQUE_RGB_COLOURS = 256 * 256 * 256;
	```
- The name of functions returning a boolean value should start with `is` or `has`; whichever makes the most sense.
	```c++
	//Correct
	bool isEven(int num) {
		return num % 2 == 0;
	}

	//Incorrect
	bool even(int num) {
		return num % 2 == 0;
	}
	```
- When a function parameter is being passed by const referance, put one space between the type and `const&`. If a parameter is passed by mutable reference do not put a space between the type and the `&`
	```c++
	//Correct for a const reference
	bool hasLargeNumber(std::vector<int> const& numbers) {
		//some cool stuff
		return true;
	}

	//Incorrect for a const reference
	bool hasLargeNumber(std::vector<int>const& numbers) {
		//some cool stuff
		return true;
	}

	//Correct for a mutable reference
	bool hasLargeNumber(std::vector<int>& numbers) {
		//some cool stuff
		return true;
	}

	//Incorrect for a mutable reference
	bool hasLargeNumber(std::vector<int> & numbers) {
		//some cool stuff
		return true;
	}
	```
-  The include order should be: C++ standard headers, followed by Godot headers, then OpenVic2 headers
	- C++ standard headers and Godot headers should be enclosed by angle brackets `<>`
	- OpenVic2 headers should be enclosed by double quotes `""`
	```c++
	//Correct
	#include <vector>
	#include <string>
	#include <godot_cpp/classes/object.hpp>
	#include <godot_cpp/core/class_db.hpp>
	#include "openvic2/src/SampleHeader.hpp"
	#include "openvic2/src/SampleHeader2.hpp"

	//Incorrect
	#include "openvic2/src/SampleHeader.hpp"
	#include <godot_cpp/classes/object.hpp>
	#include "openvic2/src/SampleHeader2.hpp"
	#include <godot_cpp/core/class_db.hpp>
	#include <vector>
	#include <string>
	```
