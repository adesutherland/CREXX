## Language levels

The \crexx{} Project is a comprehensive initiative aimed at advancing the \rexx{} language. It envisions developing a modern, ground-up implementation of a \rexx{} interpreter and compiler, along with experimenting with language enhancements. One crucial aspect of the project is to revisit and evaluate potential improvements to the \rexx{} language, while preserving its fundamental essence: making programming more accessible than ever before.

\crexx{} is designed to run on various platforms, including VM/370 (paying homage to \rexx{}'s heritage), Linux, Windows, OSX, and z/Architecture. The project will encompass different levels or flavors of REXX, each serving a specific purpose:

## Level A: PoC Precursor

Level A was a proof-of-concept precursor and will not be actively developed further.

## Level B: Foundation for Other \crexx{} Levels

Level B serves as the foundational layer for other cREXX levels. It can be viewed as a system language, and many of the cREXX components will be implemented in this level. Level B's approach emphasizes low-level built-in functions directly implemented in the bytecode interpreter. Other levels with complete sets of REXX capabilities and functions will be implemented in REXX Level B.

## Level C: Classic \rexx{}

Level C embodies "Classic" rexx{}. It aims to adhere as closely as possible to the ANSI standards and the \rexx{} implementation found on the IBM Mainframe family.

## Level D: Extended Language Features

Level D builds upon Level C by introducing new language features, such as the USE statement, to enhance the capabilities of the language.

## Level E: OO\rexx{}

Level E encompasses OO\rexx{}, an object-oriented extension of \rexx{}. While it is not anticipated to be directly delivered by the \crexx{} project, it acknowledges its significance in the \rexx{} ecosystem.

## Level G: Modern General-Purpose \rexx{}

Level G represents a modern, general-purpose \rexx{} that extends Level B syntax and offers contemporary language capabilities.

## Level L: Specialist \rexx{} for Language Engineering

Level L is a specialized REXX tailored for computer language engineering. A significant portion of the cREXX components will ultimately be implemented in REXX Level L.

# Overview of \crexx{} Level B

## \crexx{} Level B: A Foundation for Safe and Efficient Programming

\crexx{} Level B is a typed programming language designed for enhanced safety and efficiency. It balances these qualities with a user-friendly experience through type inference. As the foundation for other \crexx{} levels, Level B serves as a system language for implementing core functionalities.

Key Features

* Type Safety. Ensures data integrity and prevents runtime errors through a robust type system.
* Type Inference. Simplifies code by automatically inferring types, maintaining a user-friendly experience.
* System Language. Forms the basis for other \crexx{} levels, providing essential building blocks.
* Low-Level Functions. Offers direct access to low-level operations for performance-critical tasks.
* Complete \rexx{} Capabilities. Future levels will build upon Level B, providing the full range of \rexx{} features and functionalities.
* Rich Development Tools. Supports compiling, assembling, disassembling, debugging, linking, packaging an running for efficient development.

Syntax

* Line continuation, labels, and comments for clear and readable code.
* Modules, namespaces, global procedures, and variables for organizing and managing code.
* Data types, literals, variables, expression operators, and statements for building complex logic.

Benefits

* Improved Safety. Reduces the risk of errors and vulnerabilities.
* Enhanced Performance. Enables efficient execution of critical tasks.
* User-Friendly Experience. Simplifies development with type inference.
* Future-Proof. Provides a solid foundation for future cREXX advancements.

Overall, \crexx{} Level B offers a powerful and versatile programming language for system development and performance-critical applications. Its combination of safety, efficiency, and user-friendliness makes it a valuable tool for developers seeking to create robust and reliable software.

