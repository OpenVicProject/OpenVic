# OpenVic2 C++ Style Guidelines 

## Table of Contents
 1. [Why Style?](styleguide-cpp.md#why-style)

    1.1. [General Principles](styleguide-cpp.md#general-principles)

    1.2 [File Formatting](styleguide-cpp.md#file-formatting)

 2. [Conventions](styleguide-cpp.md#conventions)

    2.1. [Naming Conventions](styleguide-cpp.md#-naming-conventions)

## 1. Why Style?
You may be wondering ”Why do we need a style guide?” ”Are you trying to give me homework?”
### 1.1. General Principles

 - Prefer clarity over brevity
 - Don’t optimize prematurely
 - Avoid C-style casts

### 1.2. File Formatting
Source code files should adhere to the following:

 - Encoded in UTF-8
 - Use tabs for indentation
 - Use LF for end-of-line sequences
 - Not have any trailing whitespace (Lines which end in spaces or tabs
 - Any #include directives should be at the top of the file

## 2. Conventions
### 2.1 Naming Conventions
| Item | Writing Convention | Example |
|--|--|--|
| Class and Struct Names | Pascal Case | MyCoolExample |
| Variables and Function Names | Camel Case | myCoolExample |
| Constants, Enum Values and Preprocessor | Macro Case | MY_COOL_EXAMPLE |
| Type aliases | Snake Case | my_cool_example_t |

```c++
# pragma once 
# include < stdio .h> 
# include < iostream > 
// A comment 
constexpr size_t UNIQUE_RGB_COLOURS = 256 * 256 * 256; 
    
struct RGBColour { 
	unsigned char r ; 
    unsigned char g ; 
    unsigned char b ; 
}; 
	
bool isColourGreyscale ( RGBColour c ); 
	
class Something { 
	
};
```