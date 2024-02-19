 # Small Forth

## Introduction

A small implementation of Forth, written in C++. It isn't a true implementation, it could be best described as 'Forth Like'. It also doesn't compile to machine code. 

It is however a partially-type safe implementation. Although words (methods) do not advertise what types they expect, if the types on the stack are incorrect they will throw an exception stating this.

Whilst small, it isn't intended to produce small code. This isn't aimed at embedded system. The parameter stack, which typically would contain bytes or 32-bit values, contains a 32-bit item type (ForthDefs.h ```ElementType```) and a union containing space for each allowable element type.

This means that this version of Forth will work with integers, floats, bools and any other type that is added. It also allows objects to be defined and constructed.


## Limitations

There is no support for the following: 

* The Forth blocks system
* Forth vocabulary
* Control of the return stack (exceptions have to return back through all the subroutines, copying the exception state to prior exec states).
* The singlely-linked list dictionary format. The dictionary is currently a map.
* Multiple versions of a word. Creating a word overwrites the previous one.

## Further development

The following need to be added for this implementation to be of any use:


* Debugger
* Loading of programs via s" <filename> "
* Test script. A lot of the Forth is tested when creating Forth-based words, but a test script would be beneficial. The creation of the words for instance does not use all loop types.
* * UTF8 - ensure str lengths and slicing use code-points rather than code units. Add support via ICU library if necessary.
* ```Allot``` to allocate space in currently-created word
* array - find, insert, remove, removeat, sort
* dictionary
* set
* Standard operands on objects, for comparisons and math. n m + to leave type(n) on stack, so if n m + cannot be added, can n m n type totype + n be executed instead?
* sort ( w array -- ) where w is anonymous word.
* create execstate local variables, and move execstate variables into them
* ```INTERPRET``` is currently implemented in C++, and it should be recoded in Forth. 
  * Dictionary to gain invocable words 
  * Typesystem to be made a ref-counted object, with invocable words
  * Word dictionary to be based on a map of lists of words map<string, list<ForthWord* >>, so forgetting a word will bring previous definition back, and, overwriting a definition won't result in memory leaks
* Allow defining words to be based on second-level words - only if needed
* Circular object references - pre register a type
* does> currently just sets the CFA. It should compile code at the end of the word, and point the CFA at it.
  this will require the first element to be a pointer to a WordBodyElement containing a pointer to an XT. Currently it is a pointer to a WordBodyElement containing an XT.
  - only if needed
* QUIT
* ABORT
* Exception handling and stack freezing (ensuring out of control words don't destroy the stack, allowing code after exception handlers to function correctly)
* CreateFromStack (create a word with name from stack, allowing the automation of word creation)
* Remove ValueType enumeration in lieu of StackElement_Type

## Future Directions

Writing it in C++ as an 'interpreted' version (well, it's not compiled to bytecode or machine code) has made it easy to implement type-safety. However, large parts of this would have been easier if I had output to a byte code and written a virtual machine for it.

A future may attempt this.

## Known bugs

#### Redefining word
When redefining an existing word, currently the system will lose track of the prior definition. The definition cannot be deleted, as other DOCOL-based words may be calling it directly.
##### Solution
Implement traditional Forth behaviour. Keep the old word alive, just unreacheable via the dictionary search. Currently the dictionary is implemented as a map of word name:c++ word object. This should be replaced with a map of word:vector<c++ word object>, and only use the last entry in the vector when search for the word by its name.

## Type system

The type system is declared in TypeSystem.h. It's a singleton that is returned by TypeSystem::GetTypeSystem(). Typically in methods where it is used, it is retrieved into a local variable named pTS.

The type system supports:
* bool
* 64-bit signed integers
* double floats
* chars
* types
* XT (a function pointer)
* Binary operations type
* Pter to CFA (single element of a word body)

(Note, there are two enums defined that are identical, StackElement_Type and ValueType. These used to be different as a StackElemenmt could stored more things than a WordElement. This is no longer the case and ValueType should be removed.)

These types are typically stored in a 32-bit unsigned integer typedef-ed as ForthType:

The lower 16 bits:
* Up to 1022 correspond to the StackElement_Type enumeration
* 1023 is an invalid type 
* 1024 -> 32767 are system objects
* 32768 -> 65535 are user objects

The upper 16 bits is the indirection level:
* 0 corresponds to a straight object - this includes simple value types such as int, but also includes objects (held in C++ as a pointer).
* 1 would correspond to a pointer to a value type, or a pointer to an object (held in C++ as a pointer to a pointer)
* 2 would correspond to a pointer, to a pointer, to a value type.

creating a variable creates a pointer (with indirection level of 1):

```" hello I am a string " variable vstr```

```type``` consumes the top element from the stack and pushes a type variable, which contains the type of the consumed element. Typically would use ```dup type .``` to disover the type of the top stack element without destroying it.

## Stacks

There are three stacks accessible from Forth:
* Data stack. The standard stack which is used when typing literals in, or before mathematical operations
* Temporary stack. Used to store data from the data stack when it is not possible to process data in the normal stack without having somewhere else to store data. For example, .s uses the temporary stack.
  ```>t``` and ```<t``` are used to move data from the data stack to the temporary stack, and back
* Return stack. This is used by loops and other control statements. It can only store integers.
* ```>r``` and ```<r``` are used to move data from/to the data stack. There are also a lot of equivalent stack operators for the return stack that exist for the standard stack.

## Arrays

Dynamic arrays are implemented as an object,

To construct an int array, with 5 as the first element, 6 as the 2nd, and 7 as the third. Then output the first two elements:

```5 array_type construct
dup 6 swap append
dup 7 swap append
dup 0 swap [n] . cr
dup 1 swap [n] . cr
```
 
Static arrays are implemented as words:
 ```5 variable <array name> 5 cells allot```

 ```<array name> 1 swap +``` to move to first (0-based) element.

## Changes needed for assembly

Currently the execution state is stored in ExecState. The following would need tracking in known locations and the ExecState removed.
* The parameter stack
* The return stack
* The nested CFA stack
* The end of the dictionary (DP)
* The word currently being created 
* The address of the next machine code word to be executed (IP)

Exceptions would need to be pushed onto the stack as a string and ```QUIT``` called directly.

The built-in word, ```Exit```, which lives at the end of each compiled (second-level) word, currently just returns ```false``` to the function that is iterating over all the word list to call. Doing so without creating an exception in the ExecState (execution state) implies that the word being run should exit.

(according to some references, this ```Exit``` should actually be called ```SEMI```)

This word would need to instead pull the return address in a known location where the next instruction pointer is stored (IP), and call a word ```NEXT```. 

```NEXT``` would need to read the next instruction pointer from IP and jump directly to it (not a subroutine call). 

The built-in word, ```QUIT``` would need to be given direct control of the return stack by implementing ```RP```, so it can blank out the current execution state and restart interpreting.

The dictionary would either need implementing as a singly-linked list, or as btree or hashmap implemented in Forth.

String literals would have to be stack-based, as they are in forth. 

The user-defined object system would need implementing in actual Forth too - improvements to how a word can be access during runtime would be needed. The defined object would be need to contain a type and a default value, for each state value - currently only the first element in a word can be accessed.

## Things to try

### Hello World!
``` : helloworld [char] ! [char] d  [char] l [char] r [char] o [char] W 32 [char] o  [char] l dup  [char] e  [char] H emitall ; ```
``` helloworld ```

### Circle area
```3.14159 constant pi```
``` : circlearea dup * pi * ;```
``` 3 circlearea .```

## Glossary

* Level 1 word. This is a word that consists purely of machine code (in this implementation it is compiled C++). At the end it they should jump to ```EXIT```, to unwind the return stack, but in this implementation the C++ simply returns.
* Level 2 word. This is a word that consists of a list of other words to execute. These words may be level 1 or level 2. 
* CFA - Code Field Address. The first instruction of both level 1 and level 2 words must be pointers to executable code. Here, level-1 instructions, the first element in the body points at a build-in function code of type ```XT```. Level-2 instructions must point at ```DOCOL```.
* ```XT``` a function pointer that all built-in words must adhere to. It accepts a pointer to the current execution state, and returns a bool. It returns true if execution should continue, or false if either an exception has been thrown or this method was an implementation of ```EXIT```

## Defined Words 

* ```EXIT``` the last CFA in a level-2 word. Ends execution of ```DOCOL``` and returns to the calling word
* ```DOCOL``` level 2 words, compiled for other words, must have this as its first CFA. It ensures the rest of the body will be executed.
* ```EXECUTE``` executes the word whose pter is on the parameter stack.
* ```NEXT``` not actually implemented. For assembly version, all machine-code (level 1) words must end with this. It returns to the calling method
* ``` ` ``` push address of following word
* ```SEE``` decompile the word pointed at the tos
* ```.``` pulls the top of the stack and outputs it to the console
* ```CLEAR``` clears stack
* ```RCLEAR``` clears return stack
* ```.s``` prints all elements in stack
* ```.rs``` prints all elements in return stack
* ```.ts``` prints all elements in temporary stack
* ```<max> <start> DO xxxx LOOP ;``` as part of a colon definition, will loop over the words represented by xxxx, max-start times
* ```<max> <start> DO xxxx ( n -- ) +LOOP ;``` as part of a colon definition, will loop over the words represented by xxxx. Increments the loop counter by n
* ```BEGIN xxx (f -- ) UNTIL``` as part of a colon definition, loops until the flag is true
* ```BEGIN xxx AGAIN``` as part of a colon definition, loops indefinitely
* ```BEGIN xxxx (f -- ) WHILE yyyy REPEAT``` executes xxxx, tests the flag, and if the flag is true, continues to yyyy and repeats. Exits if the flag is false
* ```UNLOOP``` removed loop values from return stack - used when exiting a word, having to unloop first. Should be called multiple times for nested loops.
* ```I``` puts iteration variable onto stack. All loops have iteration variables - DO allows its value to be chosen, other loops start at 0
* ```LEAVE``` leaves a loop prematurely
* ```5 CONSTANT five``` creates a constant called five, that is set to the number five. Can do this with bools, floats, chars and types too
  ```five``` pushes the constant 'five' to the stack
* ```6 VARIABLE six``` creates an integer variable called six, that holds the value 6
  ```7 six !``` sets the variable six to be equal to seven
  ```six @``` reads the variable six and pushes it to the stack
* ```2.5 variable fiveovertwo``` creates a float variable containing 2.5. Can also create bool, char and type variables
  ```fiveovertwo``` pushes the point to the variable to the stack
* ``` ( f -- ) if xxx then zzz``` as part of a colon definition, executes xxx (then zzz) if f is true, otherwise executes zzz
* ``` ( f -- ) if xxx else yyy then zzz``` as part of a colon definition, executes xxx (then zzz) if f is true, otherwise executes yyy (then zzz)
* nested if/else in a colon definition :
  ``` ( f -- ) if aaa else 
      ( f -- ) if ccc else
      ( f -- ) if eee else
 	  fff
      THEN THEN THEN


     : helloworld [char] ! [char] d  [char] l [char] r [char] o [char] W space  [char] o  [char] l dup  [char] e  [char] H emitall ;
* ```NOT``` inverts binary at TOS
* ```AND```, ```XOR```, ```OR``` binary operations on the top two booleans
* ```ALLOT``` append additional space to the last defined word, allowing it to be used as a static array. (note, this has to be called immediately after defining the word)
* ```ELAPSEDSECONDS``` get the number of elapsed seconds from the operating systems high resolution timer, as a double.


## Defining an object

### Defining  a vector

```0.0 0.0 2 " vector2 " define
vector2_type :: x 0 self [n] ;;
vector2_type :: y 1 self [n] ;;
vector2_type :: x= 0 self [n]= ;;
vector2_type :: y= 1 self [n]= ;;

vector2_type :: tostring string_type construct [char] ( swap append x #s swap append [char] , swap append 32 tochar swap append y #s swap append [char] ) swap append ;;
```

0.0, 0.0 are the default state elements of the object being defined
2 is the number of state elements
" vector2 " is the name of the object being defined.
define defines this object, and a type variable vector2_type.


This can be constructed with:

```vector2_type construct
dup
1.1 swap
x=
```

x and y can be access via ```x``` and ```y``` when an vector2_type is on the top of the stack. They can be set via:

```vector2_type construct
dup 1.1 swap x=
dup 1.2 swap y=
.s
```

Note, the tostring method added to the vector2_type is called whenever the object is to be displayed.

### Defining a named vector and the garbage collector

```vector2_type construct
dup
" named vector object " 2 " namedvector " define
namedvector_type :: vec 0 self [n] ;;
namedvector_type :: name 1 self [n] ;;
namedvector_type :: tostring string_type construct name swap append [char] : swap append 32 tochar swap append vec #s swap append ;;
```

The 2 here indicates the number of state elements. The first one is a vector2 that is constructed on the first line. The second is a string literal.

Note, when refering to other objects, either user-defined objects or a built-in string, the garbage collector deletes any object that is no longer accessible from a root object.

In terms of this system, a root object is defined as either:
* An element on the stack
* An element in a word

Currently circular references are not possible, as cannot reference an object type not yet defined. However, the code should be able to deal with preventing circular references from keeping objects alive that are not reachable from a root object.