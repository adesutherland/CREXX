# Export overview

| NS               | CLASS      | TODO   | ITEM             | KIND      | RETURNTYPE | SIG                        |
|---------------- |---------- |------ |---------------- |--------- |---------- |-------------------------- |
|                  |            |        | Class library    |           |            |                            |
| crypto\_des      |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | crypto\_des      | namespace |            |                            |
|                  | des        |        | des              | class     |            |                            |
|                  | des        |        | encrypt          | method    |            | = .string                  |
|                  | des        |        | decrypt          | method    |            | = .string                  |
| crypto\_des      |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | crypto\_aes      | namespace |            |                            |
|                  | des        |        | aes              | class     |            |                            |
|                  | des        |        | encrypt          | method    |            | = .string                  |
|                  | des        |        | decrypt          | method    |            | = .string                  |
| data\_LinkedList |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | data\_LinkedList | namespace |            |                            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | LinkedList       | class     | class      |                            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add              | method    | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst         | method    | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast          | method    | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring         | method    | = .string  | =.void                     |
| data\_Stack      |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | data\_Stack      | namespace |            |                            |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Stack            | class     |            |                            |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push             | method    | = .void    | value=.string              |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop              | method    | = .string  | =.void                     |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring         | method    | = .string  | =.void                     |
| data\_TreeMap    |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | data\_TreeMap    | namespace |            |                            |
| data\_TreeMap    | TreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | TreeMap          | class     |            |                            |
| data\_TreeMap    | TreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put              | method    | = .void    | key=.string, value=.string |
| data\_TreeMap    | TreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get              | method    | = .string  | key=.string                |
| data\_TreeMap    | TreeMap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring         | method    | = .string  | =.void                     |
| system\_os       |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | system\_os       | namespace |            |                            |
| system\_os       | os         | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | os               | class     |            |                            |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime           | method    | = .int     | =.void                     |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user             | method    | = .string  | =.void                     |


# Export methods

| NS               | CLASS      | TODO   | ITEM     | RETURNTYPE | SIG                        |
|---------------- |---------- |------ |-------- |---------- |-------------------------- |
|                  | des        |        | encrypt  |            | = .string                  |
|                  | des        |        | decrypt  |            | = .string                  |
|                  | des        |        | encrypt  |            | = .string                  |
|                  | des        |        | decrypt  |            | = .string                  |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add      | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast  | = .int     | element=.string            |
| data\_LinkedList | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring | = .string  | =.void                     |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push     | = .void    | value=.string              |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop      | = .string  | =.void                     |
| data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring | = .string  | =.void                     |
| data\_TreeMap    | TreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put      | = .void    | key=.string, value=.string |
| data\_TreeMap    | TreeMap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get      | = .string  | key=.string                |
| data\_TreeMap    | TreeMap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring | = .string  | =.void                     |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime   | = .int     | =.void                     |
| system\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user     | = .string  | =.void                     |


# Export attributes

| NS | CLASS | TODO | ITEM | TYPE |
|--- |----- |---- |---- |---- |


# Export per class


## LinkedList


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


## Stack


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


## TreeMap


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
