# Class library


# Export tables


## Overview

| NS               | CLASS      | TODO   | ITEM             | KIND      | RETURNTYPE | SIG                      |
|---------------- |---------- |------ |---------------- |--------- |---------- |------------------------ |
|                  |            |        | Class library    |           |            |                          |
| Data\_LinkedList |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | Data\_LinkedList | namespace |            |                          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | LinkedList       | class     |            |                          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add              | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst         | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast          | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring         | method    | = .string  | =.void                   |
| Data\_Stack      |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Data\_Stack      | namespace |            |                          |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Stack            | class     |            |                          |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push             | method    | = .void    | value=string             |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop              | method    | = .string  | =.void                   |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring         | method    | = .string  | =.void                   |
| Data\_TreeMap    |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Data\_TreeMap    | namespace |            |                          |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | TreeMap          | class     |            |                          |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put              | method    | = .void    | key=string, value=string |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get              | method    | = .string  | key=string               |
| Data\_TreeMap    | Treemap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring         | method    | = .string  | =.void                   |
| System\_os       |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | System\_os       | namespace |            |                          |
| System\_os       | os         | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | os               | class     |            |                          |
| System\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime           | method    | = .int     | =.void                   |
| System\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user             | method    | = .string  | void                     |


# Export overview

| NS               | CLASS      | TODO   | ITEM             | KIND      | RETURNTYPE | SIG                      |
|---------------- |---------- |------ |---------------- |--------- |---------- |------------------------ |
|                  |            |        | Class library    |           |            |                          |
| Data\_LinkedList |            | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | Data\_LinkedList | namespace |            |                          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | LinkedList       | class     |            |                          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | add              | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addFirst         | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | addLast          | method    |            | element=.string          |
|                  | LinkedList | <img alt="SPEC" src="https://img.shields.io/badge/state-spec-lightgrey">   | tostring         | method    | = .string  | =.void                   |
| Data\_Stack      |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Data\_Stack      | namespace |            |                          |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Stack            | class     |            |                          |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | push             | method    | = .void    | value=string             |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | pop              | method    | = .string  | =.void                   |
| Data\_Stack      | Stack      | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | tostring         | method    | = .string  | =.void                   |
| Data\_TreeMap    |            | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | Data\_TreeMap    | namespace |            |                          |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | TreeMap          | class     |            |                          |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | put              | method    | = .void    | key=string, value=string |
| Data\_TreeMap    | Treemap    | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | get              | method    | = .string  | key=string               |
| Data\_TreeMap    | Treemap    | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | tostring         | method    | = .string  | =.void                   |
| System\_os       |            | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | System\_os       | namespace |            |                          |
| System\_os       | os         | <img alt="DESIGN" src="https://img.shields.io/badge/state-design-blue"> | os               | class     |            |                          |
| System\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | uptime           | method    | = .int     | =.void                   |
| System\_os       | os         | <img alt="BUILD" src="https://img.shields.io/badge/state-build-yellow">  | user             | method    | = .string  | void                     |


# Export methods

| NS            | CLASS      | RETURNTYPE | ITEM     | SIG                      |
|------------- |---------- |---------- |-------- |------------------------ |
|               | LinkedList |            | add      | element=.string          |
|               | LinkedList |            | addFirst | element=.string          |
|               | LinkedList |            | addLast  | element=.string          |
|               | LinkedList | = .string  | tostring | =.void                   |
| Data\_Stack   | Stack      | = .void    | push     | value=string             |
| Data\_Stack   | Stack      | = .string  | pop      | =.void                   |
| Data\_Stack   | Stack      | = .string  | tostring | =.void                   |
| Data\_TreeMap | Treemap    | = .void    | put      | key=string, value=string |
| Data\_TreeMap | Treemap    | = .string  | get      | key=string               |
| Data\_TreeMap | Treemap    | = .string  | tostring | =.void                   |
| System\_os    | os         | = .int     | uptime   | =.void                   |
| System\_os    | os         | = .string  | user     | void                     |


# Export attributes

| ITEM | NS | CLASS | TYPE |
|---- |--- |----- |---- |


# Export classes

| NS            | CLASS      | TYPE  |
|------------- |---------- |----- |
|               | LinkedList | class |
| Data\_Stack   | Stack      |       |
| Data\_TreeMap | Treemap    |       |
| System\_os    | os         |       |
