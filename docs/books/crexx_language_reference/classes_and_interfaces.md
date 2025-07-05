# Classes and Interfaces {#classes-and-interfaces}

*CURRENT STATUS: Classes are not implemented*

See also **Objects** subsection under **Data Types**.

## **Design Motivation** {#design-motivation}

### *Classes and Interfaces* {#classes-and-interfaces-1}

Classes and Interfaces in CREXX serve as templates defining the properties and methods of an object. These templates enable REXX programs to utilise objects effectively. The introduction of a structured object-oriented paradigm allows for better program organisation, encapsulation, and reusability.

### *Level B Considerations* {#level-b-considerations}

For CREXX Level B, the class design must facilitate explicit management of low-level REXX VM objects. This requirement necessitates the ability to access all of an object's physical attributes, a feature that will not be present in Level G. In contrast, Level G will offer additional capabilities such as dynamically defined classes and operator overriding. Despite these differences, the core class philosophy will be consistent across both levels.

### *Design Philosophy* {#design-philosophy}

The CREXX class system aims to provide a powerful yet simple framework that aligns with the language's overarching objective of simplicity. Object orientation, while powerful, often introduces complexity, especially in designing class hierarchies and balancing cohesion with loose coupling. CREXX addresses these challenges by adopting a simplified approach, providing mandated solutions for class definition, object creation, interfaces, and singletons. 

This approach ensures a common and predictable structure for CREXX programs, reducing the need for extensive refactoring.

### *Design Principles* {#design-principles}

1. **Leveraging Classic REXX Syntax Elements.** CREXX utilises familiar REXX syntax elements, such as stem syntax and label syntax, for defining methods. This design choice helps REXX developers transition smoothly into using object-oriented constructs without a steep learning curve.

2. **Interfaces as Key Building Blocks.** Interfaces define the methods (procedures) that any implementing class must provide. They are central to CREXX's design, promoting loose coupling by separating the definition of functionality from its implementation. This separation facilitates easier maintenance and extension of code.

3. **Default Method Implementations in Interfaces.** Interfaces can include default implementations of methods, effectively serving as abstract classes. This feature helps avoid code duplication by allowing shared functionality to be defined once in an interface rather than repeatedly in multiple classes.

4. **Singleton Interfaces.** Optionally, an interface can be declared as a singleton, ensuring that only one instance of a class implementing the interface exists at runtime. CREXX uses singletons instead of static members. Static members can cause issues as they do not promote loose coupling. In contrast, singletons provide a controlled and consistent approach to managing shared resources or global states, ensuring reliable and unified access to a single instance across the program.

5. **Class and Interface Implementation.** Classes in CREXX must implement one or more interfaces, with each class having at least an implicit interface matching the class name. This requirement ensures that all classes have a defined contract they adhere to, promoting consistency and reliability.

6. **Encapsulation of Attributes.** Classes can only expose methods; attributes are private and cannot be accessed directly. This encapsulation principle safeguards the integrity of the object's state and prevents unintended interference from external code.

7. **Unique Method and Attribute Names in Multi-Interface Classes.** For any class implementing multiple interfaces, each method and attribute must have a unique name. Given that CREXX Level B does not support polymorphism for procedures, this principle prevents naming conflicts and ambiguities. It simplifies class design and reduces the risk of errors, making the code easier to understand and maintain, and enhancing the system's overall reliability and robustness.

8. **Single File Class Definition.** To maintain simplicity and cohesion, each class or interface must be defined in a single file. Additionally, a file defining a class or interface cannot contain non-class procedures. This constraint helps prevent class bloat, ensuring that class designs remain focused and manageable, and that files are clearly organised and purpose-specific.

9. **Factory Functions for Object Creation.** Each class defines factory functions for the interfaces it implements. Objects are created using a syntax that revolves around the interface rather than the class itself. This practice promotes a loosely coupled design, making it easier to change the underlying implementation without affecting the rest of the program.

10. **Common Object Interface.** Every class in CREXX will implement a standard Object interface, which provides common capabilities across all classes. This interface includes a set of default method implementations, ensuring consistent behaviour and a uniform baseline of functionality for all objects. By standardising these core capabilities, the Object interface simplifies the development process and promotes code consistency and interoperability within the CREXX ecosystem.

11. **Consistent Error Handling with a Signal Interface**. The CREXX class system will utilise a Signal Interface to implement consistent and structured error handling across all classes. This interface provides a standard mechanism for managing exceptions and signals, ensuring that errors are captured, logged, and handled uniformly. By standardising the process of signalling exceptional conditions, this approach enhances the reliability and robustness of CREXX applications.

12. **Compatibility with Classic REXX Structures.** The design of Level B classes in CREXX must be sufficiently powerful to implement classic REXX structures, such as stems and variables, in Level C. This capability ensures that the object-oriented features introduced in Level B can seamlessly support traditional REXX data structures, maintaining consistency and usability across different levels of the language.

13. **Backward Compatibility in Class Syntax.** In anticipation of Level D, which aims to provide an updated yet backward-compatible version of classic REXX, the class syntax in CREXX should be designed with compatibility in mind. This includes ensuring that class definitions and operations can be expressed in a manner that aligns with the established syntax of classic REXX stems, facilitating an easier transition for existing REXX users and preserving the language's legacy features.

## **Interface Definition Syntax** {#interface-definition-syntax}

A single file / module can only define one interface or class and cannot define any other non-object language elements, meaning that procedures (including the implicit main procedure) and globals cannot be defined in an interface file.

### *Interface Syntax* {#interface-syntax}

Interfaces are defined using a \<**label\>** / \<**interface\>** instruction to scope the interface and \<**label\>** / \<**method\>** instructions to define methods. These follow a similar syntax to the \<**procedure\>** instruction.

The  methods define the return type (after the \=) and are followed by an \<**arg**\> instruction to define the arguments (using the same syntax as for procedures). The \<expose\> instruction cannot be used because interfaces and classes cannot access global variables (these could be encapsulated by a procedure called by a method but this is not a recommended construct for an object oriented program).

Note that method names must be unique across a class which may be implementing multiple interfaces.

an\_interface: interface 

    a\_method\_1: method \= 0.0 /\* returns a float \*/  
        arg …

    a\_method\_2: method \= .int /\* returns an int\*/  
        arg ...
### *Default Method Definitions* {#default-method-definitions}

A method can include the implementation (which is used as the default unless overridden by a class), in this case attributes can also be defined. Note that method and attribute names must be unique across a class which may be implementing multiple interfaces. 

Within a method definition, you can reference the current instance of the class implementing the interface by using the interface name itself. See Self-Reference in Methods under Class Definition Syntax.

an\_interface: interface   
an\_attribute \= .string

    	a\_method: method \= .string  
        	Return an\_attribute

### *Stem Access* {#stem-access}

Special method names \<\*\> are used to process object stem syntax calls. These can either have a return result to support “get”. In this case the arguments are the index values a, b and c.

result \= object.a.b.c  or  result \= object\[a,b,c\]

Or they can return nothing / .void to support ”set”. In this case the first argument is the new\_value, and the next arguments are index a, b and c.

object.a.b.c \= new\_value  or  object\[a,b,c\] \= new\_value

Note that as Level B does not support method overloading, different argument or return types are not supported. However the ellipse (...) format for a variable number of arguments is supported. It is envisaged that arguments will be of type .string but this is not mandated.

an\_interface: interface   
     
/\* Setter for an\_interface.a \= new\_value \*/    	  
\*: method   
		Arg new\_val \= .string, index \= .string

	/\* Getter for value \= an\_interface.index \*/  
    	\*: method \= .string   
Arg index \= .string

### *Singletons* {#singletons}

Interfaces can be defined as singletons in which case only one instance of a class implementing the interface can exist at runtime. The \<label\> / \<singleton\> instruction is used. This can be optionally followed by \<interface\> which is only provided for human readability. In addition it must be followed by “= .a\_class” which defines the default class that should be instantiated if the singleton is accessed before it has been given a value. 

a\_singleton: singleton interface \= .default\_class 

    a\_method: method \= 0.0 /\* returns a float \*/  
        arg …

Singletons are initialised by calling a relevant class provided factory (see factory methods)

a\_singleton() /\* Default Factory \*/

a\_singleton.factory\_with\_params(“initfile.txt”) /\* Maybe \*/

Multiple assignments replace the singleton instance. If a singleton is used before it is assigned the default factory will be used (or a signal raises), in this case the class is instantiated on first use.

## **Class Definition Syntax** {#class-definition-syntax}

Each file or module can define only one interface or class. It cannot contain non-object elements such as procedures (including the implicit main procedure) or global variables. This rule ensures clear separation of object-oriented components from other program elements.

Each class must implement one or more interfaces. There are two notable interfaces:

* Object Interface. Implemented by all classes, this interface provides default implementations for all its methods. The specific methods of the Object Interface are yet to be determined. This interface is implicitly included and cannot be explicitly listed in the class definition.  
* Intrinsic Interface. This interface shares the same name as the class and is automatically defined. Like the Object Interface, the Intrinsic Interface is implicit and cannot be explicitly listed in the class’s definition, or any other class’ definition.

Method and attribute names must be unique within a class, especially when implementing multiple interfaces, to prevent conflicts and ensure clarity.

## **Attribute Definition and Initialization** {#attribute-definition-and-initialization}

Attributes can be simple data types, arrays or classes and are declared / initialised within a class structure.

a\_class: class \= .interface  
  attr\_1 \= .string /\* string type \*/  
  attr\_2 \= 1 /\* Integer type and initialised to 1 \*/  
  …

In cREXX, class attributes are private and can only be accessed or modified through methods that encapsulate them. This ensures data integrity and prevents unintended external interference.

## **Physical Attribute Access** {#physical-attribute-access}

Level B requires access to specific physical attributes to allow access to physical objects (see Objects Section) defined by the CREXX VM. 

In this example the two attributes are mapped to the first two child attributes in the object. The reserve keyword specifies the attribute number being reserved / mapped to the attribute.

procedure\_object: class    /\* Using an Intrinsic Interface \*/  
  reserve.1 name \= .string    /\* string type \*/  
  reserve.2 proc\_block \= .int /\* Procedure pointer can be used in dcall \*/  
  …

Using the keyword reserve without an index means reserve the base register of the object. In the following example this register holds both the integer and string value and ASSEMBLER instructions are needed to correctly access and update the object. The syntax is designed to reserve the register so the compiler does not attempt to use it.

signal\_object: class    /\* Using an Intrinsic Interface \*/  
  reserve signal       /\* No type \- but the register is reserved \*/  
  /\* In this example ASSEMBLER instructions would be needed for access \*/    …

## **Method Definition**  {#method-definition}

### *Private Methods* {#private-methods}

A class may declare methods not included in any of the interfaces it implements (except the Intrinsic Interface). Such methods are private to the class. To maintain a loosely coupled architecture, interfaces should be explicitly defined, with the Intrinsic Interface serving as a fallback mechanism only usable in very simple cases.

### *Self-Reference in Methods* {#self-reference-in-methods}

In CREXX, within a method definition, you can reference the current instance of the class by using the class name itself. This allows for explicitness in the code when accessing attributes or calling other methods on the same object, and allows the object to be returned from methods (e.g. Factory methods).

### *Factory Methods* {#factory-methods}

Factory Methods are the primary mechanism for constructing new objects in CREXX. These methods are defined using the \<label\> / \<factory\> syntax. This approach allows for creating object instances using a syntax that focuses on the interface rather than the class itself.

* Default Factory. The \<label\> can be "\*", indicating the default factory method for the interface. Each interface can have only one default factory method, ensuring a clear and unambiguous way to instantiate objects.  
* Unique Method Names. Factory method names must be unique across all classes implementing a particular interface. This restriction also ensures that only one class can define a default factory for an interface. Additionally, when a class implements multiple interfaces, the factory method names must be unique across all the interfaces. This ensures that object initialization can rely solely on the interface name and the factory method name.

By adhering to these conventions, CREXX maintains a structured and consistent approach to object creation, simplifying the process for developers and avoiding conflicts that could arise from ambiguous method naming.

For example the following class definition part

a\_class: class \= interface1

	\*: factory /\* Default Factory \*/

		Return a\_class

	from\_string: factory

		arg name \= .string

		… /\* create the object \*/

		return a\_class

An instance can be created with the following statements

instance1 \= .interface1

Instance2 \= .interface1.from\_string(“something”)

Instance3 \= .interface1.something\_else() /\* From a different class \*/

## **Syntax to use Interfaces and Classes** {#syntax-to-use-interfaces-and-classes}

### *Calling a method* {#calling-a-method}

an\_object \= an\_interface /\* Default Factory \*/

an\_object.a\_method() /\* Calls Method \*/

### *Getter / Setter* {#getter-/-setter}

An\_object \= “value”; say an\_object

### *Getter / Setter with Stem. format* {#getter-/-setter-with-stem.-format}

an\_object.index \= value; say an\_object.index

### *Initialising a Singleton* {#initialising-a-singleton}

config.fileinit(“initfile.txt”) /\* Maybe \*/

### *Accessing a Singleton* {#accessing-a-singleton}

Currency \= config.default\_currency /\* Example \*/

## **Class / Interface Example** {#class-/-interface-example}

### *Interface* {#interface}

/\* Define an interface named \`Vehicle\` \*/  
vehicle: interface  
/\* Start and stop the vehicle \*/  
start: method  
stop: method

/\* Set the vehicle name \*/  
\*: method   
Arg new\_val \= .string

/\* Get vehicle name \*/  
\*: method \= .string 		

### *Car Class* {#car-class}

/\* Define a class \`Car\` implementing the \`Vehicle\` interface \*/  
car: class \= .vehicle  
  car\_name \= .string   
  range \= .float	  
     
  /\* Default vehicle is a car :-( \*/  
  \*: factory  
    /\* Create and return an instance of Car \*/  
    range \= 350 /\* miles \*/  
    car\_name \= “anonymous”  
    return car

  /\* Named factory method \*/  
  new\_car: factory  
    arg name \= .string  
    range \= 350 /\* miles \*/  
    car\_name \= name  
    return car

  /\* Start and stop the vehicle \*/  
  start: method  
	range \= range \- 100;

  stop: method  
	/\* Do something \*/

  /\* Set the vehicle name \*/  
  \*: method   
Arg new\_val \= .string  
Car\_name \= new\_val

  /\* Get vehicle name \*/   
  \*: method \= .string   
	return car\_name

### *Bike Class* {#bike-class}

/\* Define a class \`bike\` implementing the \`Vehicle\` interface \*/  
bike: class \= .vehicle

  /\* Named factory method \*/  
  new\_bike: factory  
    return bike

  /\* Start and stop the vehicle \*/  
  start: method  
	/\* Do something \*/

  stop: method  
	/\* Do something \*/

  /\* Set the vehicle name \*/  
  \*: method   
Arg new\_val \= .string  
/\* Do nothing as bikes don’t have names in this world \*/  
/\* Could raise a signal \*/

  /\* Get vehicle name \*/   
  \*: method \= .string   
	return “Just a Bike” 

### *Usage* {#usage}

car\_instance \= .vehicle /\* Uses the default factory method in \`Car\` \*/

car\_instance\_named \= .vehicle.new\_car("Sedan")

say car\_instance\_named /\* Returns “Sedan” \*/ 

bike\_instance \= .vehicle.new\_bike  /\* A bike \*/

say bike\_instance /\* Returns “Just a Bike” \*/ 

