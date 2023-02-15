# OpenVic2 C++ Style Guidelines

## Table of Contents

- [Why Style?](#why-style)
- [1. Conventions](#1-conventions)
  - [1.1. General Principles](#11-general-principles)
  - [1.2. File Formatting](#12-file-formatting)
  - [1.3. Naming Conventions](#13-naming-conventions)
  - [1.4. Do’s and Don’ts](#14-dos-and-donts)
    - [1.4.1. Keep the opening scope brace on the same
      line](#141-keep-the-opening-scope-brace-on-the-same-line)
    - [1.4.2. Keep spaces between arithmetic operators, relational
      operators, logical operators, variables, and
      literals](#142-keep-spaces-between-arithmetic-operators-relational-operators-logical-operators-variables-and-literals)
    - [1.4.3. Prefix and postfix operators should be immediately
      attached to the
      variable](#143-prefix-and-postfix-operators-should-be-immediately-attached-to-the-variable)
    - [1.4.4. Avoid `inline`, use Godot’s `_FORCE_INLINE_` for release
      inline behavior and `_ALWAYS_INLINE_` for always inline
      behavior](#144-avoid-inline-use-godots-_force_inline_-for-release-inline-behavior-and-_always_inline_-for-always-inline-behavior)
    - [1.4.5. Avoid function definitions in header files, prefer
      definitions in implementation
      files](#145-avoid-function-definitions-in-header-files-prefer-definitions-in-implementation-files)
    - [1.4.6. The content of classes should appear in the following
      order:](#146-the-content-of-classes-should-appear-in-the-following-order)
    - [1.4.7. Class visibility specifiers should be the same indentation
      of the
      class](#147-class-visibility-specifiers-should-be-the-same-indentation-of-the-class)
    - [1.4.8. Class visibility specifiers should be followed by a
      declaration on a new line at the next indentation
      level](#148-class-visibility-specifiers-should-be-followed-by-a-declaration-on-a-new-line-at-the-next-indentation-level)
    - [1.4.9. Methods which do not mutate an object’s state should be
      marked as a `const`
      method](#149-methods-which-do-not-mutate-an-objects-state-should-be-marked-as-a-const-method)
    - [1.4.10. When argument or parameter lists are too long to easily
      fit on one line: begin the argument list on the next line, two
      indentation levels higher than the function’s
      indentation.](#1410-when-argument-or-parameter-lists-are-too-long-to-easily-fit-on-one-line-begin-the-argument-list-on-the-next-line-two-indentation-levels-higher-than-the-functions-indentation)
    - [1.4.11. In header files, put `#pragma once` at the top of the
      file, not C-style header
      guards](#1411-in-header-files-put-pragma-once-at-the-top-of-the-file-not-c-style-header-guards)
    - [1.4.12. Use enum classes, not plain
      enums](#1412-use-enum-classes-not-plain-enums)
    - [1.4.13. Use type aliases, not
      \`typedef\`s](#1413-use-type-aliases-not-typedefs)
    - [1.4.14. Use explicit types, do not use
      `auto`](#1414-use-explicit-types-do-not-use-auto)
    - [1.4.15. Do not use C-style casts, instead use C++ casts (ie:
      `static_cast`, `const_cast`,
      `reinterpret_cast`)](#1415-do-not-use-c-style-casts-instead-use-c-casts-ie-static_cast-const_cast-reinterpret_cast)
    - [1.4.16. When declaring a pointer/reference, put a space between
      the type and the asterisk/ambersand, put the asterisk/ambersand
      immediately before the variable
      name.](#1416-when-declaring-a-pointerreference-put-a-space-between-the-type-and-the-asteriskambersand-put-the-asteriskambersand-immediately-before-the-variable-name)
    - [1.4.17. Do not declare multiple pointer variables on a single
      line](#1417-do-not-declare-multiple-pointer-variables-on-a-single-line)
    - [1.4.18. Keep the initialization of a variable and its declaration
      as close as possible, ideally on the same
      line.](#1418-keep-the-initialization-of-a-variable-and-its-declaration-as-close-as-possible-ideally-on-the-same-line)
    - [1.4.19. For constant values that are computable at compile-time,
      use `constexpr` instead of the `const`
      keyword](#1419-for-constant-values-that-are-computable-at-compile-time-use-constexpr-instead-of-the-const-keyword)
    - [1.4.20. The name of functions returning a boolean value should
      start with `is` or `has`; whichever makes the most
      sense.](#1420-the-name-of-functions-returning-a-boolean-value-should-start-with-is-or-has-whichever-makes-the-most-sense)
    - [1.4.21. When a function parameter is being passed by const
      referance, put one space between the type and `const&` and then
      another space between `const&` and the variable name. If a
      parameter is passed by mutable reference do not put a space
      between the `&` and the variable
      name.](#1421-when-a-function-parameter-is-being-passed-by-const-referance-put-one-space-between-the-type-and-const-and-then-another-space-between-const-and-the-variable-name-if-a-parameter-is-passed-by-mutable-reference-do-not-put-a-space-between-the--and-the-variable-name)
    - [1.4.22. Headers](#1422-headers)
    - [1.4.23. Prefer Godot’s Variant types over Godot’s template types
      and the Standard
      Library](#1423-prefer-godots-variant-types-over-godots-template-types-and-the-standard-library)
    - [1.4.24. Prefer `godot::String` and `godot::StringName` to
      `std::string` and
      `std::string_view`](#1424-prefer-godotstring-and-godotstringname-to-stdstring-and-stdstring_view)
    - [1.4.25. Prefer Godot template types over the Standard Template
      Library](#1425-prefer-godot-template-types-over-the-standard-template-library)
    - [1.4.26. Do not use Exceptions](#1426-do-not-use-exceptions)
    - [1.4.27. Do not use RTTI](#1427-do-not-use-rtti)

## Why Style?

> “Programs are meant to be read by humans and only incidentally for
> computers to execute.”

— H. Abelson and G. Sussman in [**Structure and Interpretation of
Computer
Programs**](https://en.wikipedia.org/wiki/Structure_and_Interpretation_of_Computer_Programs)

The purpose of this styleguide is to give a clear framework for other
contributors, to minimize ambiguity, and to have a consistant way to
convey the intent of code.

## 1. Conventions

### 1.1. General Principles

1.  Prefer clarity over brevity

2.  [Don’t optimize prematurely](https://youtu.be/tKbV6BpH-C8)

3.  In cases of ambiguity, use your best judgement

### 1.2. File Formatting

Source code files should adhere to the following:

1.  Encoded in UTF-8

2.  Use tabs for indentation, only use one tab per indent level

3.  Use LF for end-of-line sequences

4.  No trailing whitespace (Lines which end in spaces or tabs)

5.  Any `#include` directives should be at the top of the file

6.  Any header files have the `.hpp` file extension

7.  Any implementation files have the `.cpp` file extension

8.  Code should be within the `OpenVic2` namespace (or a nested
    namespace)

9.  Any submodules or third-party utilities should be in the
    `OpenVic2/extension/deps` directory

10. Groups of related source files should be kept in a sub-directory of
    `OpenVic2/extension/src`

11. Any lines that are longer than 120 characters in length should be
    split across multiple lines

12. Use British English spellings and standards.

### 1.3. Naming Conventions

|        |                                   |                                     |                                         |
|--------|-----------------------------------|-------------------------------------|-----------------------------------------|
| Number | Item                              | Writing Convention                  | Example                                 |
| 1      | Class names                       | PascalCase                          | MyCoolExample                           |
| 2      | Struct names                      | PascalCase                          | MyCoolExample                           |
| 3      | Variable names                    | Lower snake_case                    | my_cool_example                         |
| 4      | Function Variable names           | Lower snake_case (`p_` prefix)      | p_my_cool_example                       |
| 5      | Private Variable names            | Lower snake_case (`_` suffix)       | \_my_cool_example                       |
| 6      | Function and Method names         | Lower snake_case                    | my_cool_example                         |
| 7      | Private Function and Method names | Lower snake_case (`_` suffix)       | \_my_cool_example                       |
| 8      | Constants                         | Upper SNAKE_CASE                    | MY_COOL_EXAMPLE                         |
| 9      | Macros                            | Upper SNAKE_CASE                    | MY_COOL_EXAMPLE                         |
| 10     | Class or container type aliases   | PascalCase                          | MyCoolExample                           |
| 11     | Builtin or numeric type aliases   | PamelCase ( `_t` suffix)            | MyCoolExample_t                         |
| 12     | Enum names                        | PascalCase                          | MyCoolExample                           |
| 13     | Enum values                       | Upper SNAKE_CASE                    | MY_COOL_EXAMPLE                         |
| 14     | Filenames for classes             | PascalCase following the class name | MyCoolExample.hpp/MyCoolExample.cpp     |
| 15     | Filenames                         | Lower snake_case                    | my_cool_example.hpp/my_cool_example.cpp |

### 1.4. Do’s and Don’ts

#### 1.4.1. Keep the opening scope brace on the same line

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

#### 1.4.2. Keep spaces between arithmetic operators, relational operators, logical operators, variables, and literals

```c++
//Correct
int my_number = (some_variable + 10) % 3;

//Incorrect
int my_number=(some_variable+10)%3;
```

#### 1.4.3. Prefix and postfix operators should be immediately attached to the variable

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

#### 1.4.4. Avoid `inline`, use Godot’s `_FORCE_INLINE_` for release inline behavior and `_ALWAYS_INLINE_` for always inline behavior

Header

`OpenVic2/godot-cpp/include/godot-cpp/core/defs.hpp`

##### Why?

Godot has inbuilt macros for more compiler consistent inline behavior,
as well inline can make debugging a lot more difficult and so should
generally be avoided in debug most of the time.

```c++
//Correct
_FORCE_INLINE_ void inline_in_release_build() {}
_ALWAYS_INLINE_ void inline_in_release_and_debug_build() {}

//Incorrect
inline void inline_in_release_and_debug_build() {}
```

#### 1.4.5. Avoid function definitions in header files, prefer definitions in implementation files

```c++
//In Example.hpp
class Example {
public:
    //Prefer
    void declared_in_header();

    //Avoid
    void defined_in_header() {}
};

//In Example.cpp
Example::declared_in_header() {}
```

#### 1.4.6. The content of classes should appear in the following order:

1.  Type Aliases

2.  Static (class-wide) Member Variables

3.  Members Variables

4.  Constructors and Destructors

5.  Static (class-wide) Methods

6.  Methods

```c++
class Human {
public:
    using name_t = std::string;

    name_t name;
    size_t age;
    size_t family_members;

    void cool_public_method();
    void another_cool_public_method();

    _FORCE_INLINE_ bool is_voting_age() const {
        return age >= 18;
    }

private:
    static size_t _number_of_humans;

    Human();
    ~Human();

    static bool is_more_humans_than(size_t number);

    void cool_private_method();
};
```

#### 1.4.7. Class visibility specifiers should be the same indentation of the class

```c++
class ExampleClass {
public:
    int example_public_variable;

protected:
    int example_protected_variable;

private:
    int example_private_variable;
};
```

#### 1.4.8. Class visibility specifiers should be followed by a declaration on a new line at the next indentation level

- Group methods and variables by their visibility specifier from least
  to most restrictive (`public`, `protected`, then `private`)

  - Reason: The public interface should be the most commonly sought
    element and should be the most easily and readily accessible if
    possible.

- Keep declarations/definitions grouped by visibility except in the case
  that a declaration/definition is dependent on another

```c++
class ExampleClass {
public:
    void first_public_method();
    void second_public_method();
    void third_public_method();

protected:
    void first_protected_method();
    void second_protected_method();
    void third_protected_method();

private:
    void first_private_method();
    void second_private_method();
    void third_private_method();
};
```

#### 1.4.9. Methods which do not mutate an object’s state should be marked as a `const` method

```c++
//Correct
bool RGBColour::is_colour_black() const {
    return red_channel == 0 && green_channel == 0 && blue_channel == 0;
}

//Incorrect
bool RGBColour::is_colour_black() {
    return red_channel == 0 && green_channel == 0 && blue_channel == 0;
}
```

#### 1.4.10. When argument or parameter lists are too long to easily fit on one line: begin the argument list on the next line, two indentation levels higher than the function’s indentation.

```c++
//Correct
calling_a_function_with_a_very_long_name_and_many_arguments(
        p_long_argument_name_one, p_long_argument_name_two, p_long_argument_name_three,
        p_long_argument_name_four, p_long_argument_name_five, p_long_argument_name_six,
        p_long_argument_name_seven, p_long_argument_name_eight);

//Incorrect
calling_a_function_with_a_very_long_name_and_many_arguments(p_long_argument_name_one, p_long_argument_name_two, p_long_argument_name_three, p_long_argument_name_four, p_long_argument_name_five, p_long_argument_name_six, p_long_argument_name_seven, p_long_argument_name_eight);
```

#### 1.4.11. In header files, put `#pragma once` at the top of the file, not C-style header guards

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

#### 1.4.12. Use enum classes, not plain enums

```c++
//Correct
enum class Colours { RED, GREEN, BLUE, CHARCOAL_GREY };

//Incorrect
enum Colours { RED, GREEN, BLUE, CHARCOAL_GREY };
```

#### 1.4.13. Use type aliases, not \`typedef\`s

```c++
//Correct
using MyContainer = std::vector<int>;
using byte_t = unsigned char;

//Incorrect
typedef std::vector<int> MyContainer;
typedef unsigned char byte_t;
```

#### 1.4.14. Use explicit types, do not use `auto`

```c++
//Correct
godot::Vector2 coordinate = get_player_location();

//Incorrect
auto coordinate = get_player_location();
```

#### 1.4.15. Do not use C-style casts, instead use C++ casts (ie: `static_cast`, `const_cast`, `reinterpret_cast`)

- `dynamic_cast` should only be used with caution

```c++
//Correct
int my_number = static_cast<int>(some_float_value);

//Incorrect
int my_number = (int) some_float_value;
```

#### 1.4.16. When declaring a pointer/reference, put a space between the type and the asterisk/ambersand, put the asterisk/ambersand immediately before the variable name.

```c++
//Correct
int *my_pointer = &my_number;

//Incorrect
int* my_pointer = &my_number;
int *my_pointer = &my_number;
int*my_pointer = &my_number;
```

#### 1.4.17. Do not declare multiple pointer variables on a single line

```c++
//Correct
int *p1;
int *p2;

//Incorrect
int *p1, *p2;
```

#### 1.4.18. Keep the initialization of a variable and its declaration as close as possible, ideally on the same line.

```c++
//Correct
int my_number = 23;

//Incorrect
int my_number;
my_number = 23;
```

#### 1.4.19. For constant values that are computable at compile-time, use `constexpr` instead of the `const` keyword

```c++
//Correct
constexpr size_t UNIQUE_RGB_COLOURS = 256 * 256 * 256;

//Incorrect
const size_t UNIQUE_RGB_COLOURS = 256 * 256 * 256;
```

#### 1.4.20. The name of functions returning a boolean value should start with `is` or `has`; whichever makes the most sense.

```c++
//Correct
bool is_even(int p_num) {
    return p_num % 2 == 0;
}

//Incorrect
bool even(int p_num) {
    return p_num % 2 == 0;
}
```

#### 1.4.21. When a function parameter is being passed by const referance, put one space between the type and `const&` and then another space between `const&` and the variable name. If a parameter is passed by mutable reference do not put a space between the `&` and the variable name.

```c++
//Correct for a const reference
bool has_large_number(std::vector<int> const& p_numbers) {
    //some cool stuff
    return true;
}

//Incorrect for a const reference
bool has_large_number(std::vector<int>const &p_numbers) {
    //some cool stuff
    return true;
}

//Correct for a mutable reference
bool has_large_number(std::vector<int> &p_numbers) {
    //some cool stuff
    return true;
}

//Incorrect for a mutable reference
bool has_large_number(std::vector<int> & p_numbers) {
    //some cool stuff
    return true;
}
```

#### 1.4.22. Headers

Order

1.  Related implementation header

2.  C standard headers

3.  C++ standard headers

4.  Godot headers

5.  OpenVic2 dependency headers

6.  OpenVic2 headers

Do’s and Don’ts

1.  Related implementation headers should be enclosed by double quotes
    `""`

    - Related implementation headers should be relative to its directory
      and should always reside in the same directory.

    - Related implementation headers should be separated from other
      headers.

2.  C standard headers, C++ standard headers, and Godot headers should
    be enclosed by angle brackets `<>`

3.  OpenVic2 dependency headers and OpenVic2 should be enclosed by
    double quotes `""`

```c++
//Correct
#include "Implementation.hpp"

#include <stdio>
#include <vector>
#include <string>
#include <godot_cpp/classes/object.hpp>
#include <godot_cpp/core/class_db.hpp>
#include ""
#include "openvic2/src/SampleHeader.hpp"
#include "openvic2/src/SampleHeader2.hpp"

//Incorrect
#include "openvic2/src/SampleHeader.hpp"
#include <godot_cpp/classes/object.hpp>
#include "openvic2/src/SampleHeader2.hpp"
#include <stdio>
#include <godot_cpp/core/class_db.hpp>
#include <vector>
#include <string>
#include "Implementation.hpp"
```

#### 1.4.23. Prefer Godot’s Variant types over Godot’s template types and the Standard Library

##### Why?

Godot has a concept of Variants stored at runtime, these variant types
are designed to integrate well into Godot and GDScript. These types are
also designed to be efficient, generic, copy-on-write, and easy to use.
They also come with convenient methods to streamline development. Many
of them are well documented in the [Godot
Docs](https://docs.godotengine.org/en/latest/) and have inbuilt support
in the editor itself.

|        |                                                                   |                                                                             |                                                                                                                                                                                                                                 |                                                                                                                                                                                                                                                                                                                            |
|--------|-------------------------------------------------------------------|-----------------------------------------------------------------------------|---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------|
| Number | Standard Type                                                     | Godot Type                                                                  | Godot Header (`OpenVic2/godot-cpp`)                                                                                                                                                                                             | [Godot Docs](https://docs.godotengine.org/en/latest)                                                                                                                                                                                                                                                                       |
| 1      | `std::string` and `std::string_view`                              | `godot::String` or `godot::StringName` or `godot::NodePath`                 | `godot-cpp/gen/include/godot_cpp/variant/string.hpp` or `godot-cpp/gen/include/godot_cpp/variant/string_name.hpp` or `godot-cpp/gen/include/godot_cpp/variant/node_path.hpp`                                                    | [String](https://docs.godotengine.org/en/latest/classes/class_string.html) or [StringName](https://docs.godotengine.org/en/latest/classes/class_stringname.html) or [NodePath](https://docs.godotengine.org/en/latest/classes/class_nodepath.html)                                                                         |
| 2      | `std::map` or `std::unordered_map` or `godot::HashMap`            | `godot::Dictionary`                                                         | `godot-cpp/gen/include/godot_cpp/variant/dictionary.hpp`                                                                                                                                                                        | [Dictionary](https://docs.godotengine.org/en/latest/classes/class_dictionary.html)                                                                                                                                                                                                                                         |
| 3      | `std::vector<T>` or `godot::Vector<T>` or `godot::LocalVector<T>` | `godot::Array`                                                              | `godot-cpp/gen/include/godot_cpp/variant/array.hpp`                                                                                                                                                                             | [Array](https://docs.godotengine.org/en/latest/classes/class_array.html)                                                                                                                                                                                                                                                   |
| 4      | `std::function`                                                   | `godot::Callable`                                                           | `godot-cpp/gen/include/godot_cpp/variant/callable.hpp`                                                                                                                                                                          | [Callable](https://docs.godotengine.org/en/latest/classes/class_callable.html)                                                                                                                                                                                                                                             |
| 5      | `std::pair<int, int>` or `std::tuple<int, int>`                   | `godot::Vector2i`                                                           | `godot-cpp/gen/include/godot_cpp/variant/vector2i.hpp`                                                                                                                                                                          | [Vector2i](https://docs.godotengine.org/en/latest/classes/class_vector2i.html)                                                                                                                                                                                                                                             |
| 6      | `std::pair<float, float>` or `std::tuple<float, float>`           | `godot::Vector2`                                                            | `godot-cpp/gen/include/godot_cpp/variant/vector2.hpp`                                                                                                                                                                           | [Vector2](https://docs.godotengine.org/en/latest/classes/class_vector2i.html)                                                                                                                                                                                                                                              |
| 7      | `std::tuple<int, int, int>`                                       | `godot::Vector3i`                                                           | `godot-cpp/gen/include/godot_cpp/variant/vector3i.hpp`                                                                                                                                                                          | [Vector3i](https://docs.godotengine.org/en/latest/classes/class_vector3i.html)                                                                                                                                                                                                                                             |
| 8      | `std::tuple<float, float, float>`                                 | `godot::Vector3`                                                            | `godot-cpp/gen/include/godot_cpp/variant/vector3.hpp`                                                                                                                                                                           | [Vector3](https://docs.godotengine.org/en/latest/classes/class_vector3.html)                                                                                                                                                                                                                                               |
| 9      | `std::tuple<int, int, int, int>`                                  | `godot::Vector4i` or `godot::Rect2i` or `godot::Color`                      | `godot-cpp/gen/include/godot_cpp/variant/vector4i.hpp` or `godot-cpp/gen/include/godot_cpp/variant/rect2i.hpp` or `godot-cpp/gen/include/godot_cpp/variant/color.hpp`                                                           | [Vector4i](https://docs.godotengine.org/en/latest/classes/class_vector4i.html) or [Rect2i](https://docs.godotengine.org/en/latest/classes/class_rect2i.html) or [Color](https://docs.godotengine.org/en/latest/classes/class_color.html)                                                                                   |
| 10     | `std::tuple<float, float, float, float>`                          | `godot::Vector4` or `godot::Rect2` or `godot::Quaternion` or `godot::Color` | `godot-cpp/gen/include/godot_cpp/variant/vector4.hpp` or `godot-cpp/gen/include/godot_cpp/variant/rect2.hpp` or `godot-cpp/gen/include/godot_cpp/variant/quaternion.hpp` or `godot-cpp/gen/include/godot_cpp/variant/color.hpp` | [Vector4](https://docs.godotengine.org/en/latest/classes/class_vector4.html) or [Rect2](https://docs.godotengine.org/en/latest/classes/class_rect2.html) or [Quaternion](https://docs.godotengine.org/en/latest/classes/class_quaternion.html) or [Color](https://docs.godotengine.org/en/latest/classes/class_color.html) |
| 11     | `std::vector<uint8_t>`                                            | `godot::PackedByteArray`                                                    | `godot-cpp/gen/include/godot_cpp/variant/packed_byte_array.hpp`                                                                                                                                                                 | [PackedByteArray](https://docs.godotengine.org/en/latest/classes/class_packedbytearray.html)                                                                                                                                                                                                                               |
| 12     | `std::vector<int>`                                                | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_int32_array.hpp`                                                                                                                                                                | [PackedInt32Array](https://docs.godotengine.org/en/latest/classes/class_packedint32array.html)                                                                                                                                                                                                                             |
| 13     | `std::vector<long>`                                               | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_int64_array.hpp`                                                                                                                                                                | [PackedInt64Array](https://docs.godotengine.org/en/latest/classes/class_packedint64array.html)                                                                                                                                                                                                                             |
| 14     | `std::vector<float>`                                              | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_float32_array.hpp`                                                                                                                                                              | [PackedFloat32Array](https://docs.godotengine.org/en/latest/classes/class_packedfloat32array.html)                                                                                                                                                                                                                         |
| 15     | `std::vector<double>`                                             | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_float64_array.hpp`                                                                                                                                                              | [PackedFloat64Array](https://docs.godotengine.org/en/latest/classes/class_packedfloat64array.html)                                                                                                                                                                                                                         |
| 16     | `std::vector<godot::String>`                                      | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_string_array.hpp`                                                                                                                                                               | [PackedStringArray](https://docs.godotengine.org/en/latest/classes/class_packedstringarray.html)                                                                                                                                                                                                                           |
| 17     | `std::vector<godot::Color>`                                       | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_color_array.hpp`                                                                                                                                                                | [PackedColorArray](https://docs.godotengine.org/en/latest/classes/class_packedcolorarray.html)                                                                                                                                                                                                                             |
| 18     | `std::vector<godot::Vector2>`                                     | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_vector2_array.hpp`                                                                                                                                                              | [PackedVector2Array](https://docs.godotengine.org/en/latest/classes/class_packedvector2array.html)                                                                                                                                                                                                                         |
| 19     | `std::vector<godot::Vector3>`                                     | `godot::PackedInt32Array`                                                   | `godot-cpp/gen/include/godot_cpp/variant/packed_vector3_array.hpp`                                                                                                                                                              | [PackedVector3Array](https://docs.godotengine.org/en/latest/classes/class_packedvector3array.html)                                                                                                                                                                                                                         |
| 20     | `std::variant<T…​>`                  							  | `godot::Object` or `godot::Variant`                                         | `godot-cpp/gen/include/godot_cpp/variant/object.hpp` or `godot-cpp/gen/include/godot_cpp/variant/variant.hpp`                                                                                                                   | [Object](https://docs.godotengine.org/en/latest/classes/class_object.html) or [Variant](https://docs.godotengine.org/en/latest/classes/class_variant.html)                                                                                                                                                                 |

#### 1.4.24. Prefer [`godot::String`](https://docs.godotengine.org/en/latest/classes/class_string.html) and [`godot::StringName`](https://docs.godotengine.org/en/latest/classes/class_stringname.html) to `std::string` and `std::string_view`

godot::String Header

`godot-cpp/gen/include/godot_cpp/variant/string.hpp`

godot::StringName Header

`godot-cpp/gen/include/godot_cpp/variant/string_name.hpp`

##### Why?

Godot’s strings have been especially designed for use in Godot with
inbuilt utility methods to ease game development and they are already
well integrated into GDScript. Godot’s strings are also optimized with
support for Unicode and performs reference-counting and copy-on-write
behavior. (which means modifications generates new strings instead of
modifying existing ones) This means they are also cheap to pass around
and can generally be treated as constant references.

StringNames are interned strings that act much like Godot’s normal
Strings, but this means each instance of a StringName that shares the
same contents will share the same pointer and so its pointer can be
compared instead of its contents. StringNames will generally convert
Strings automatically, they otherwise act like godot::String. It is more
expensive to construct StringNames.

#### 1.4.25. Prefer Godot template types over the Standard Template Library

##### Why?

Godot’s template types are designed to be efficient, reduce debugging
frustrations, and reduce template bloat common with the STL. See the
[Godot Docs on not using the
STL](https://docs.godotengine.org/en/latest/about/faq.html#why-does-godot-not-use-stl-standard-template-library).

|        |                                    |                                                               |                                                                            |
|--------|------------------------------------|---------------------------------------------------------------|----------------------------------------------------------------------------|
| Number | Standard Type                      | Godot Type                                                    | Godot Header (`OpenVic2/godot-cpp/include`)                                |
| 1      | `std::vector<T>`                   | `godot::Vector<T>` or `godot::LocalVector<T>`                 | `godot-cpp/templates/vector.hpp` or `godot-cpp/templates/local_vector.hpp` |
| 2      | `std::list<T>`                     | `godot::List<T>`                                              | `godot-cpp/templates/list.hpp`                                             |
| 3      | `std::unordered_map<TKey, TValue>` | `godot::HashMap<TKey, TValue>`                                | `godot-cpp/templates/hash_map.hpp`                                         |
| 4      | `std::set<TKey, TValue>`           | `godot::RBSet<TKey, TValue>` or `godot:HashSet<TKey, TValue>` | `godot-cpp/templates/rb_set.hpp` or `godot-cpp/templates/hash_set.hpp`     |

#### 1.4.26. Do not use Exceptions

##### Why?

Exceptions have many problems, they can harm performance and bloat
binaries. Exceptions also treat crashing as the program’s default
failure behavior. This is undesirable and even explictly designed
against in Godot itself and makes debugging with the Godot editor more
difficult. As well Godot has many built in manners to address program
errors that makes throwing exceptions practically moot while making
issues easier to track in the Godot editor. As for why Godot opposes
exceptions see the [Godot Docs on not using
exceptions](https://docs.godotengine.org/en/latest/about/faq.html#why-does-godot-not-use-exceptions).

#### 1.4.27. Do not use RTTI

##### Why?

Like exceptions, RTTI has a cost, most specifically increasing the
binary size. Godot comes inbuilt with its own type system with its own
type-casting so using RTTI should be made moot.
