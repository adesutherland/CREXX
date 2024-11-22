# Introduction

The cREXX Project is a comprehensive initiative aimed at advancing the REXX language. It envisions developing a modern, ground-up implementation of a REXX interpreter and compiler, along with experimenting with language enhancements. One crucial aspect of the project is to revisit and evaluate potential improvements to the REXX language, while preserving its fundamental essence: making programming more accessible than ever before.

cREXX is designed to run on various platforms, including VM/370 (paying homage to REXX's heritage), Linux, Windows, OSX, and z/Architecture. The project will encompass different levels or flavors of REXX, each serving a specific purpose:

## **Level A: PoC Precursor**

Level A was a proof-of-concept precursor and will not be actively developed further.

## **Level B: Foundation for Other cREXX Levels**

Level B serves as the foundational layer for other cREXX levels. It can be viewed as a system language, and many of the cREXX components will be implemented in this level. Level B's approach emphasizes low-level built-in functions directly implemented in the bytecode interpreter. Other levels with complete sets of REXX capabilities and functions will be implemented in REXX Level B.

## **Level C: Classic REXX**

Level C embodies "Classic" REXX. It aims to adhere as closely as possible to the ANSI standards and the REXX implementation found on the IBM Mainframe family.

## **Level D: Extended Language Features**

Level D builds upon Level C by introducing new language features, such as the USE statement, to enhance the capabilities of the language.

## **Level E: OORexx**

Level E encompasses OORexx, an object-oriented extension of REXX. While it is not anticipated to be directly delivered by the cREXX project, it acknowledges its significance in the REXX ecosystem.

## **Level G: Modern General-Purpose REXX**

Level G represents a modern, general-purpose REXX that extends Level B syntax and offers contemporary language capabilities.

## **Level L: Specialist REXX for Language Engineering**

Level L is a specialized REXX tailored for computer language engineering. A significant portion of the cREXX components will ultimately be implemented in REXX Level L.

## **About this Document**

This document serves as a Language Reference for cREXX Level B as of the June 2024 snapshots. It is an ongoing document, subject to revisions and improvements. The cREXX project welcomes feedback and corrections from the community to ensure the accuracy and completeness of this resource.

# Overview of cREXX Level B

## **cREXX Level B: A Foundation for Safe and Efficient Programming**

cREXX Level B is a typed programming language designed for enhanced safety and efficiency. It balances these qualities with a user-friendly experience through type inference. As the foundation for other cREXX levels, Level B serves as a system language for implementing core functionalities.

**Key Features**

* **Type Safety.** Ensures data integrity and prevents runtime errors through a robust type system.  
* **Type Inference.** Simplifies code by automatically inferring types, maintaining a user-friendly experience.  
* **System Language.** Forms the basis for other cREXX levels, providing essential building blocks.  
* **Low-Level Functions.** Offers direct access to low-level operations for performance-critical tasks.  
* **Complete REXX Capabilities.** Future levels will build upon Level B, providing the full range of REXX features and functionalities.  
* **Rich Development Tools.** Supports compiling, assembling, disassembling, debugging, and packaging for efficient development.

**Syntax**

* Line continuation, labels, and comments for clear and readable code.  
* Modules, namespaces, global procedures, and variables for organizing and managing code.  
* Data types, literals, variables, expression operators, and statements for building complex logic.

**Benefits**

* **Improved Safety.** Reduces the risk of errors and vulnerabilities.  
* **Enhanced Performance.** Enables efficient execution of critical tasks.  
* **User-Friendly Experience.** Simplifies development with type inference.  
* **Future-Proof.** Provides a solid foundation for future cREXX advancements.

Overall, cREXX Level B offers a powerful and versatile programming language for system development and performance-critical applications. Its combination of safety, efficiency, and user-friendliness makes it a valuable tool for developers seeking to create robust and reliable software.

