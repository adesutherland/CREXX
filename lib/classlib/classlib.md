# Export overview

Collection class names are contract-tagged in Level B because the language does
not yet have generics: `String...` stores string values, `Object...` stores
object values, and `StringObject...` maps string keys to object values. Bare
collection names are reserved for future Level G generic or generic-like
surfaces.

The concrete class source files and `CMakeLists.txt` entries are the source of
truth for currently built exports. The planning tables below are retained as an
export checklist and may still contain legacy namespace labels from the
pre-generic naming pass.

| NS               | CLASS      | TODO   | ITEM             | KIND      | RETURNTYPE | SIG                        |
|---------------- |---------- |------ |---------------- |--------- |---------- |-------------------------- |
|                  |            |        | Class library    |           |            |                            |
| crypto\_des      |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | crypto\_des      | namespace |            |                            |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | des              | class     |            |                            |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | encrypt          | method    |            | = .string                  |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | decrypt          | method    |            | = .string                  |
| crypto\_aes      |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | crypto\_aes      | namespace |            |                            |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | aes              | class     |            |                            |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | encrypt          | method    |            | = .string                  |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | decrypt          | method    |            | = .string                  |
| data\_LinkedList |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | data\_LinkedList | namespace |            |                            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | StringLinkedList       | class     | class      |                            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add              | method    | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst         | method    | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast          | method    | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring         | method    | = .string  | =.void                     |
| data\_Stack      |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | data\_Stack      | namespace |            |                            |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | StringStack            | class     |            |                            |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push             | method    | = .void    | value=.string              |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop              | method    | = .string  | =.void                     |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring         | method    | = .string  | =.void                     |
| data\_TreeMap    |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | data\_TreeMap    | namespace |            |                            |
| data\_TreeMap    | StringTreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | StringTreeMap          | class     |            |                            |
| data\_TreeMap    | StringTreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put              | method    | = .void    | key=.string, value=.string |
| data\_TreeMap    | StringTreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get              | method    | = .string  | key=.string                |
| data\_TreeMap    | StringTreeMap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring         | method    | = .string  | =.void                     |
| rxmath\_math     |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | rxmath.math      | namespace |            |                            |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | math             | class     |            |                            |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | pi               | method    |            | = .void                    |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | sin              | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | square           | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | pow              | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | base             | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | cosin            | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tan              | method    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | arctan           | method    |            | = .float                   |
| system\_ipc      |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | system\_ipc      | namespace |            |                            |
|                  | semaphore  | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | semaphore        | class     |            |                            |
|                  | pipe       | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | pipe             | class     |            |                            |
|                  | ClassName  | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | queue            | class     |            |                            |
| system\_os       |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | system\_os       | namespace |            |                            |
| system\_os       | os         | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | os               | class     |            |                            |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime           | method    | = .int     | =.void                     |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user             | method    | = .string  | =.void                     |


# Export methods

| NS               | CLASS      | TODO   | ITEM     | RETURNTYPE | SIG                        |
|---------------- |---------- |------ |-------- |---------- |-------------------------- |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | encrypt  |            | = .string                  |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | decrypt  |            | = .string                  |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | encrypt  |            | = .string                  |
|                  | des        | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | decrypt  |            | = .string                  |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add      | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast  | = .int     | element=.string            |
| data\_LinkedList | StringLinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring | = .string  | =.void                     |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push     | = .void    | value=.string              |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop      | = .string  | =.void                     |
| data\_Stack      | StringStack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring | = .string  | =.void                     |
| data\_TreeMap    | StringTreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put      | = .void    | key=.string, value=.string |
| data\_TreeMap    | StringTreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get      | = .string  | key=.string                |
| data\_TreeMap    | StringTreeMap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring | = .string  | =.void                     |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | pi       |            | = .void                    |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | sin      |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | square   |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | pow      |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | base     |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | cosin    |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tan      |            | = .float                   |
|                  | math       | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | arctan   |            | = .float                   |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime   | = .int     | =.void                     |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user     | = .string  | =.void                     |


# Export attributes

| NS | CLASS | TODO | ITEM | TYPE |
|--- |----- |---- |---- |---- |


# Export per class


## StringLinkedList


### Overview

| TODO | ITEM | KIND | RETURNTYPE | SIG |
|---- |---- |---- |---------- |--- |


### Methods

| TODO | ITEM     | RETURNTYPE | SIG             |
|---- |-------- |---------- |--------------- |
| <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey"> | add      | = .int     | element=.string |
| <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey"> | addFirst | = .int     | element=.string |
| <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey"> | addLast  | = .int     | element=.string |
| <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey"> | tostring | = .string  | =.void          |


### Attributes

| TODO | ITEM | TYPE |
|---- |---- |---- |


## StringStack


### Overview

| TODO | ITEM | KIND | RETURNTYPE | SIG |
|---- |---- |---- |---------- |--- |


### Methods

| TODO  | ITEM     | RETURNTYPE | SIG           |
|----- |-------- |---------- |------------- |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow"> | push     | = .void    | value=.string |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow"> | pop      | = .string  | =.void        |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow"> | tostring | = .string  | =.void        |


### Attributes

| TODO | ITEM | TYPE |
|---- |---- |---- |


## StringTreeMap


### Overview

| TODO | ITEM | KIND | RETURNTYPE | SIG |
|---- |---- |---- |---------- |--- |


### Methods

| TODO   | ITEM     | RETURNTYPE | SIG                        |
|------ |-------- |---------- |-------------------------- |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put      | = .void    | key=.string, value=.string |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get      | = .string  | key=.string                |
| <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring | = .string  | =.void                     |


### Attributes

| TODO | ITEM | TYPE |
|---- |---- |---- |


## os


### Overview

| TODO | ITEM | KIND | RETURNTYPE | SIG |
|---- |---- |---- |---------- |--- |


### Methods

| TODO  | ITEM   | RETURNTYPE | SIG    |
|----- |------ |---------- |------ |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow"> | uptime | = .int     | =.void |
| <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow"> | user   | = .string  | =.void |


### Attributes

| TODO | ITEM | TYPE |
|---- |---- |---- |
