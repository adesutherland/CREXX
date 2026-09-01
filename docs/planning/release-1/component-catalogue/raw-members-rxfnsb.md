# Raw public class and interface members: rxfnsb

Stage 2 discovery data. A member is included only when its containing class or interface is namespace-exposed in the same source file.

| ID | Namespace | Owner | Owner kind | Member | Member kind | Declared result/status | Source |
|---|---|---|---|---|---|---|---|
| `MEM-rxsysb-addressbinding-default` | `_rxsysb` | `addressbinding` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:174` |
| `MEM-rxsysb-addressbinding-get-external-alias` | `_rxsysb` | `addressbinding` | class | `get_external_alias` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:190` |
| `MEM-rxsysb-addressbinding-get-flags` | `_rxsysb` | `addressbinding` | class | `get_flags` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:212` |
| `MEM-rxsysb-addressbinding-get-internal-name` | `_rxsysb` | `addressbinding` | class | `get_internal_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:187` |
| `MEM-rxsysb-addressbinding-get-kind` | `_rxsysb` | `addressbinding` | class | `get_kind` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:184` |
| `MEM-rxsysb-addressbinding-get-stem-value` | `_rxsysb` | `addressbinding` | class | `get_stem_value` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:199` |
| `MEM-rxsysb-addressbinding-get-value` | `_rxsysb` | `addressbinding` | class | `get_value` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:193` |
| `MEM-rxsysb-addressbinding-get-value-object` | `_rxsysb` | `addressbinding` | class | `get_value_object` | method | `= .addressstem` | `lib/rxfnsb/rexx/_address.crexx:196` |
| `MEM-rxsysb-addressbinding-set-stem-value` | `_rxsysb` | `addressbinding` | class | `set_stem_value` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:205` |
| `MEM-rxsysb-addressdriverregistry-default` | `_rxsysb` | `addressdriverregistry` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:36` |
| `MEM-rxsysb-addressdriverregistry-add` | `_rxsysb` | `addressdriverregistry` | class | `add` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:40` |
| `MEM-rxsysb-addressdriverregistry-drivers` | `_rxsysb` | `addressdriverregistry` | class | `drivers` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:74` |
| `MEM-rxsysb-addressdriverregistry-lookup` | `_rxsysb` | `addressdriverregistry` | class | `lookup` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:50` |
| `MEM-rxsysb-addressenvironment-default` | `_rxsysb` | `addressenvironment` | interface | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:593` |
| `MEM-rxsysb-addressenvironment-environment-id` | `_rxsysb` | `addressenvironment` | interface | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:598` |
| `MEM-rxsysb-addressenvironment-environment-name` | `_rxsysb` | `addressenvironment` | interface | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:596` |
| `MEM-rxsysb-addressenvironment-execute` | `_rxsysb` | `addressenvironment` | interface | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:600` |
| `MEM-rxsysb-addressfunctionenvironment-invoke` | `_rxsysb` | `addressfunctionenvironment` | interface | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:589` |
| `MEM-rxsysb-addressfunctionrequest-default` | `_rxsysb` | `addressfunctionrequest` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:509` |
| `MEM-rxsysb-addressfunctionrequest-get-argument` | `_rxsysb` | `addressfunctionrequest` | class | `get_argument` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:527` |
| `MEM-rxsysb-addressfunctionrequest-get-argument-count` | `_rxsysb` | `addressfunctionrequest` | class | `get_argument_count` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:524` |
| `MEM-rxsysb-addressfunctionrequest-get-environment-name` | `_rxsysb` | `addressfunctionrequest` | class | `get_environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:518` |
| `MEM-rxsysb-addressfunctionrequest-get-flags` | `_rxsysb` | `addressfunctionrequest` | class | `get_flags` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:535` |
| `MEM-rxsysb-addressfunctionrequest-get-function-name` | `_rxsysb` | `addressfunctionrequest` | class | `get_function_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:521` |
| `MEM-rxsysb-addressfunctionrequest-get-sandbox` | `_rxsysb` | `addressfunctionrequest` | class | `get_sandbox` | method | `= .addresssandbox` | `lib/rxfnsb/rexx/_address.crexx:532` |
| `MEM-rxsysb-addressfunctionresponse-default` | `_rxsysb` | `addressfunctionresponse` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:545` |
| `MEM-rxsysb-addressfunctionresponse-add-diagnostic` | `_rxsysb` | `addressfunctionresponse` | class | `add_diagnostic` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:574` |
| `MEM-rxsysb-addressfunctionresponse-get-condition-name` | `_rxsysb` | `addressfunctionresponse` | class | `get_condition_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:567` |
| `MEM-rxsysb-addressfunctionresponse-get-diagnostic` | `_rxsysb` | `addressfunctionresponse` | class | `get_diagnostic` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:584` |
| `MEM-rxsysb-addressfunctionresponse-get-diagnostic-count` | `_rxsysb` | `addressfunctionresponse` | class | `get_diagnostic_count` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:581` |
| `MEM-rxsysb-addressfunctionresponse-get-rc` | `_rxsysb` | `addressfunctionresponse` | class | `get_rc` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:553` |
| `MEM-rxsysb-addressfunctionresponse-get-result` | `_rxsysb` | `addressfunctionresponse` | class | `get_result` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:560` |
| `MEM-rxsysb-addressfunctionresponse-set-condition-name` | `_rxsysb` | `addressfunctionresponse` | class | `set_condition_name` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:570` |
| `MEM-rxsysb-addressfunctionresponse-set-rc` | `_rxsysb` | `addressfunctionresponse` | class | `set_rc` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:556` |
| `MEM-rxsysb-addressfunctionresponse-set-result` | `_rxsysb` | `addressfunctionresponse` | class | `set_result` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:563` |
| `MEM-rxsysb-addressinstance-bind-environment` | `_rxsysb` | `addressinstance` | interface | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:495` |
| `MEM-rxsysb-addressinstance-environment-id` | `_rxsysb` | `addressinstance` | interface | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:500` |
| `MEM-rxsysb-addressinstance-environment-name` | `_rxsysb` | `addressinstance` | interface | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:498` |
| `MEM-rxsysb-addressrequest-default` | `_rxsysb` | `addressrequest` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:307` |
| `MEM-rxsysb-addressrequest-add-binding` | `_rxsysb` | `addressrequest` | class | `add_binding` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:377` |
| `MEM-rxsysb-addressrequest-add-binding-plan` | `_rxsysb` | `addressrequest` | class | `add_binding_plan` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:388` |
| `MEM-rxsysb-addressrequest-add-stem-binding-plan` | `_rxsysb` | `addressrequest` | class | `add_stem_binding_plan` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:381` |
| `MEM-rxsysb-addressrequest-get-binding` | `_rxsysb` | `addressrequest` | class | `get_binding` | method | `= .addressbinding` | `lib/rxfnsb/rexx/_address.crexx:398` |
| `MEM-rxsysb-addressrequest-get-binding-count` | `_rxsysb` | `addressrequest` | class | `get_binding_count` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:395` |
| `MEM-rxsysb-addressrequest-get-binding-stem-value` | `_rxsysb` | `addressrequest` | class | `get_binding_stem_value` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:419` |
| `MEM-rxsysb-addressrequest-get-binding-value` | `_rxsysb` | `addressrequest` | class | `get_binding_value` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:402` |
| `MEM-rxsysb-addressrequest-get-command` | `_rxsysb` | `addressrequest` | class | `get_command` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:326` |
| `MEM-rxsysb-addressrequest-get-environment-name` | `_rxsysb` | `addressrequest` | class | `get_environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:319` |
| `MEM-rxsysb-addressrequest-get-flags` | `_rxsysb` | `addressrequest` | class | `get_flags` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:354` |
| `MEM-rxsysb-addressrequest-get-sandbox` | `_rxsysb` | `addressrequest` | class | `get_sandbox` | method | `= .addresssandbox` | `lib/rxfnsb/rexx/_address.crexx:357` |
| `MEM-rxsysb-addressrequest-get-sandbox-value` | `_rxsysb` | `addressrequest` | class | `get_sandbox_value` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:364` |
| `MEM-rxsysb-addressrequest-get-stderr-endpoint` | `_rxsysb` | `addressrequest` | class | `get_stderr_endpoint` | method | `= .binary` | `lib/rxfnsb/rexx/_address.crexx:347` |
| `MEM-rxsysb-addressrequest-get-stdin-endpoint` | `_rxsysb` | `addressrequest` | class | `get_stdin_endpoint` | method | `= .binary` | `lib/rxfnsb/rexx/_address.crexx:333` |
| `MEM-rxsysb-addressrequest-get-stdout-endpoint` | `_rxsysb` | `addressrequest` | class | `get_stdout_endpoint` | method | `= .binary` | `lib/rxfnsb/rexx/_address.crexx:340` |
| `MEM-rxsysb-addressrequest-set-binding-stem-value` | `_rxsysb` | `addressrequest` | class | `set_binding_stem_value` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:425` |
| `MEM-rxsysb-addressrequest-set-command` | `_rxsysb` | `addressrequest` | class | `set_command` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:329` |
| `MEM-rxsysb-addressrequest-set-environment-name` | `_rxsysb` | `addressrequest` | class | `set_environment_name` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:322` |
| `MEM-rxsysb-addressrequest-set-sandbox` | `_rxsysb` | `addressrequest` | class | `set_sandbox` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:360` |
| `MEM-rxsysb-addressrequest-set-sandbox-value` | `_rxsysb` | `addressrequest` | class | `set_sandbox_value` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:370` |
| `MEM-rxsysb-addressrequest-set-stderr-endpoint` | `_rxsysb` | `addressrequest` | class | `set_stderr_endpoint` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:350` |
| `MEM-rxsysb-addressrequest-set-stdin-endpoint` | `_rxsysb` | `addressrequest` | class | `set_stdin_endpoint` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:336` |
| `MEM-rxsysb-addressrequest-set-stdout-endpoint` | `_rxsysb` | `addressrequest` | class | `set_stdout_endpoint` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:343` |
| `MEM-rxsysb-addressresponse-default` | `_rxsysb` | `addressresponse` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:440` |
| `MEM-rxsysb-addressresponse-add-diagnostic` | `_rxsysb` | `addressresponse` | class | `add_diagnostic` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:462` |
| `MEM-rxsysb-addressresponse-add-updated-binding` | `_rxsysb` | `addressresponse` | class | `add_updated_binding` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:476` |
| `MEM-rxsysb-addressresponse-add-updated-binding-plan` | `_rxsysb` | `addressresponse` | class | `add_updated_binding_plan` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:480` |
| `MEM-rxsysb-addressresponse-get-condition-name` | `_rxsysb` | `addressresponse` | class | `get_condition_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:455` |
| `MEM-rxsysb-addressresponse-get-diagnostic` | `_rxsysb` | `addressresponse` | class | `get_diagnostic` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:472` |
| `MEM-rxsysb-addressresponse-get-diagnostic-count` | `_rxsysb` | `addressresponse` | class | `get_diagnostic_count` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:469` |
| `MEM-rxsysb-addressresponse-get-rc` | `_rxsysb` | `addressresponse` | class | `get_rc` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:448` |
| `MEM-rxsysb-addressresponse-get-updated-binding` | `_rxsysb` | `addressresponse` | class | `get_updated_binding` | method | `= .addressbinding` | `lib/rxfnsb/rexx/_address.crexx:490` |
| `MEM-rxsysb-addressresponse-get-updated-binding-count` | `_rxsysb` | `addressresponse` | class | `get_updated_binding_count` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:487` |
| `MEM-rxsysb-addressresponse-set-condition-name` | `_rxsysb` | `addressresponse` | class | `set_condition_name` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:458` |
| `MEM-rxsysb-addressresponse-set-rc` | `_rxsysb` | `addressresponse` | class | `set_rc` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:451` |
| `MEM-rxsysb-addresssandbox-drop` | `_rxsysb` | `addresssandbox` | interface | `drop` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:222` |
| `MEM-rxsysb-addresssandbox-exists` | `_rxsysb` | `addresssandbox` | interface | `exists` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:225` |
| `MEM-rxsysb-addresssandbox-get` | `_rxsysb` | `addresssandbox` | interface | `get` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:216` |
| `MEM-rxsysb-addresssandbox-next` | `_rxsysb` | `addresssandbox` | interface | `next` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:228` |
| `MEM-rxsysb-addresssandbox-set` | `_rxsysb` | `addresssandbox` | interface | `set` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:219` |
| `MEM-rxsysb-addressstem-drop` | `_rxsysb` | `addressstem` | interface | `drop` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:92` |
| `MEM-rxsysb-addressstem-exists` | `_rxsysb` | `addressstem` | interface | `exists` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:95` |
| `MEM-rxsysb-addressstem-get` | `_rxsysb` | `addressstem` | interface | `get` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:86` |
| `MEM-rxsysb-addressstem-next` | `_rxsysb` | `addressstem` | interface | `next` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:98` |
| `MEM-rxsysb-addressstem-set` | `_rxsysb` | `addressstem` | interface | `set` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:89` |
| `MEM-rxsysb-crexxaddressenvironment-default` | `_rxsysb` | `crexxaddressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:612` |
| `MEM-rxsysb-crexxaddressenvironment-bind-environment` | `_rxsysb` | `crexxaddressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:618` |
| `MEM-rxsysb-crexxaddressenvironment-environment-id` | `_rxsysb` | `crexxaddressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:628` |
| `MEM-rxsysb-crexxaddressenvironment-environment-name` | `_rxsysb` | `crexxaddressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:625` |
| `MEM-rxsysb-crexxaddressenvironment-execute` | `_rxsysb` | `crexxaddressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:631` |
| `MEM-rxsysb-crexxaddressenvironment-invoke` | `_rxsysb` | `crexxaddressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:635` |
| `MEM-rxsysb-nativeaddressenvironment-default` | `_rxsysb` | `nativeaddressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:797` |
| `MEM-rxsysb-nativeaddressenvironment-bind-environment` | `_rxsysb` | `nativeaddressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:805` |
| `MEM-rxsysb-nativeaddressenvironment-environment-id` | `_rxsysb` | `nativeaddressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:816` |
| `MEM-rxsysb-nativeaddressenvironment-environment-name` | `_rxsysb` | `nativeaddressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:813` |
| `MEM-rxsysb-nativeaddressenvironment-execute` | `_rxsysb` | `nativeaddressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:819` |
| `MEM-rxsysb-nativeaddressenvironment-invoke` | `_rxsysb` | `nativeaddressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:828` |
| `MEM-rxsysb-pathaddressenvironment-default` | `_rxsysb` | `pathaddressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:722` |
| `MEM-rxsysb-pathaddressenvironment-bind-environment` | `_rxsysb` | `pathaddressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:728` |
| `MEM-rxsysb-pathaddressenvironment-environment-id` | `_rxsysb` | `pathaddressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:738` |
| `MEM-rxsysb-pathaddressenvironment-environment-name` | `_rxsysb` | `pathaddressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:735` |
| `MEM-rxsysb-pathaddressenvironment-execute` | `_rxsysb` | `pathaddressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:741` |
| `MEM-rxsysb-pathaddressenvironment-invoke` | `_rxsysb` | `pathaddressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:745` |
| `MEM-rxsysb-shelladdressenvironment-default` | `_rxsysb` | `shelladdressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:686` |
| `MEM-rxsysb-shelladdressenvironment-bind-environment` | `_rxsysb` | `shelladdressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:692` |
| `MEM-rxsysb-shelladdressenvironment-environment-id` | `_rxsysb` | `shelladdressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:702` |
| `MEM-rxsysb-shelladdressenvironment-environment-name` | `_rxsysb` | `shelladdressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:699` |
| `MEM-rxsysb-shelladdressenvironment-execute` | `_rxsysb` | `shelladdressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:705` |
| `MEM-rxsysb-shelladdressenvironment-invoke` | `_rxsysb` | `shelladdressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:709` |
| `MEM-rxsysb-standardaddresssandbox-default` | `_rxsysb` | `standardaddresssandbox` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:237` |
| `MEM-rxsysb-standardaddresssandbox-drop` | `_rxsysb` | `standardaddresssandbox` | class | `drop` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:267` |
| `MEM-rxsysb-standardaddresssandbox-exists` | `_rxsysb` | `standardaddresssandbox` | class | `exists` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:276` |
| `MEM-rxsysb-standardaddresssandbox-get` | `_rxsysb` | `standardaddresssandbox` | class | `get` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:241` |
| `MEM-rxsysb-standardaddresssandbox-next` | `_rxsysb` | `standardaddresssandbox` | class | `next` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:281` |
| `MEM-rxsysb-standardaddresssandbox-set` | `_rxsysb` | `standardaddresssandbox` | class | `set` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:248` |
| `MEM-rxsysb-standardaddressstem-default` | `_rxsysb` | `standardaddressstem` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:107` |
| `MEM-rxsysb-standardaddressstem-drop` | `_rxsysb` | `standardaddressstem` | class | `drop` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:137` |
| `MEM-rxsysb-standardaddressstem-exists` | `_rxsysb` | `standardaddressstem` | class | `exists` | method | `= .int` | `lib/rxfnsb/rexx/_address.crexx:146` |
| `MEM-rxsysb-standardaddressstem-get` | `_rxsysb` | `standardaddressstem` | class | `get` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:111` |
| `MEM-rxsysb-standardaddressstem-next` | `_rxsysb` | `standardaddressstem` | class | `next` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:151` |
| `MEM-rxsysb-standardaddressstem-set` | `_rxsysb` | `standardaddressstem` | class | `set` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:118` |
| `MEM-rxsysb-systemaddressenvironment-default` | `_rxsysb` | `systemaddressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:650` |
| `MEM-rxsysb-systemaddressenvironment-bind-environment` | `_rxsysb` | `systemaddressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:656` |
| `MEM-rxsysb-systemaddressenvironment-environment-id` | `_rxsysb` | `systemaddressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:666` |
| `MEM-rxsysb-systemaddressenvironment-environment-name` | `_rxsysb` | `systemaddressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:663` |
| `MEM-rxsysb-systemaddressenvironment-execute` | `_rxsysb` | `systemaddressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:669` |
| `MEM-rxsysb-systemaddressenvironment-invoke` | `_rxsysb` | `systemaddressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:673` |
| `MEM-rxsysb-unknownaddressenvironment-default` | `_rxsysb` | `unknownaddressenvironment` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/_address.crexx:757` |
| `MEM-rxsysb-unknownaddressenvironment-bind-environment` | `_rxsysb` | `unknownaddressenvironment` | class | `bind_environment` | method | `= .void` | `lib/rxfnsb/rexx/_address.crexx:763` |
| `MEM-rxsysb-unknownaddressenvironment-environment-id` | `_rxsysb` | `unknownaddressenvironment` | class | `environment_id` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:773` |
| `MEM-rxsysb-unknownaddressenvironment-environment-name` | `_rxsysb` | `unknownaddressenvironment` | class | `environment_name` | method | `= .string` | `lib/rxfnsb/rexx/_address.crexx:770` |
| `MEM-rxsysb-unknownaddressenvironment-execute` | `_rxsysb` | `unknownaddressenvironment` | class | `execute` | method | `= .addressresponse` | `lib/rxfnsb/rexx/_address.crexx:776` |
| `MEM-rxsysb-unknownaddressenvironment-invoke` | `_rxsysb` | `unknownaddressenvironment` | class | `invoke` | method | `= .addressfunctionresponse` | `lib/rxfnsb/rexx/_address.crexx:783` |
| `MEM-rxfnsb-runtime-signal-default` | `rxfnsb` | `runtime_signal` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:119` |
| `MEM-rxfnsb-runtime-signal-address` | `rxfnsb` | `runtime_signal` | class | `address` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:144` |
| `MEM-rxfnsb-runtime-signal-code` | `rxfnsb` | `runtime_signal` | class | `code` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:136` |
| `MEM-rxfnsb-runtime-signal-column` | `rxfnsb` | `runtime_signal` | class | `column` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:162` |
| `MEM-rxfnsb-runtime-signal-file` | `rxfnsb` | `runtime_signal` | class | `file` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:156` |
| `MEM-rxfnsb-runtime-signal-line` | `rxfnsb` | `runtime_signal` | class | `line` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:159` |
| `MEM-rxfnsb-runtime-signal-message` | `rxfnsb` | `runtime_signal` | class | `message` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:148` |
| `MEM-rxfnsb-runtime-signal-module` | `rxfnsb` | `runtime_signal` | class | `module` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:140` |
| `MEM-rxfnsb-runtime-signal-name` | `rxfnsb` | `runtime_signal` | class | `name` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:132` |
| `MEM-rxfnsb-runtime-signal-payload` | `rxfnsb` | `runtime_signal` | class | `payload` | method | `= .object` | `lib/rxfnsb/rexx/signal.crexx:152` |
| `MEM-rxfnsb-runtime-signal-set-raw` | `rxfnsb` | `runtime_signal` | class | `set_raw` | method | `= .void` | `lib/rxfnsb/rexx/signal.crexx:126` |
| `MEM-rxfnsb-runtime-signal-source` | `rxfnsb` | `runtime_signal` | class | `source` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:165` |
| `MEM-rxfnsb-runtime-signal-raw-address` | `rxfnsb` | `runtime_signal_raw` | class | `address` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:103` |
| `MEM-rxfnsb-runtime-signal-raw-code` | `rxfnsb` | `runtime_signal_raw` | class | `code` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:97` |
| `MEM-rxfnsb-runtime-signal-raw-message` | `rxfnsb` | `runtime_signal_raw` | class | `message` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:106` |
| `MEM-rxfnsb-runtime-signal-raw-module` | `rxfnsb` | `runtime_signal_raw` | class | `module` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:100` |
| `MEM-rxfnsb-runtime-signal-raw-name` | `rxfnsb` | `runtime_signal_raw` | class | `name` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:94` |
| `MEM-rxfnsb-runtime-signal-raw-payload` | `rxfnsb` | `runtime_signal_raw` | class | `payload` | method | `= .object` | `lib/rxfnsb/rexx/signal.crexx:109` |
| `MEM-rxfnsb-signal-default` | `rxfnsb` | `signal` | interface | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:31` |
| `MEM-rxfnsb-signal-address` | `rxfnsb` | `signal` | interface | `address` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:37` |
| `MEM-rxfnsb-signal-code` | `rxfnsb` | `signal` | interface | `code` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:35` |
| `MEM-rxfnsb-signal-column` | `rxfnsb` | `signal` | interface | `column` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:42` |
| `MEM-rxfnsb-signal-file` | `rxfnsb` | `signal` | interface | `file` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:40` |
| `MEM-rxfnsb-signal-line` | `rxfnsb` | `signal` | interface | `line` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:41` |
| `MEM-rxfnsb-signal-message` | `rxfnsb` | `signal` | interface | `message` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:38` |
| `MEM-rxfnsb-signal-module` | `rxfnsb` | `signal` | interface | `module` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:36` |
| `MEM-rxfnsb-signal-name` | `rxfnsb` | `signal` | interface | `name` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:34` |
| `MEM-rxfnsb-signal-payload` | `rxfnsb` | `signal` | interface | `payload` | method | `= .object` | `lib/rxfnsb/rexx/signal.crexx:39` |
| `MEM-rxfnsb-signal-source` | `rxfnsb` | `signal` | interface | `source` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:43` |
| `MEM-rxfnsb-signalaction-fail` | `rxfnsb` | `signalaction` | interface | `fail` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:171` |
| `MEM-rxfnsb-signalaction-kind` | `rxfnsb` | `signalaction` | interface | `kind` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:172` |
| `MEM-rxfnsb-signalaction-retry` | `rxfnsb` | `signalaction` | interface | `retry` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:169` |
| `MEM-rxfnsb-signalaction-skip` | `rxfnsb` | `signalaction` | interface | `skip` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:170` |
| `MEM-rxfnsb-standard-signal-default` | `rxfnsb` | `standard_signal` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:50` |
| `MEM-rxfnsb-standard-signal-address` | `rxfnsb` | `standard_signal` | class | `address` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:65` |
| `MEM-rxfnsb-standard-signal-code` | `rxfnsb` | `standard_signal` | class | `code` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:59` |
| `MEM-rxfnsb-standard-signal-column` | `rxfnsb` | `standard_signal` | class | `column` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:80` |
| `MEM-rxfnsb-standard-signal-file` | `rxfnsb` | `standard_signal` | class | `file` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:74` |
| `MEM-rxfnsb-standard-signal-line` | `rxfnsb` | `standard_signal` | class | `line` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:77` |
| `MEM-rxfnsb-standard-signal-message` | `rxfnsb` | `standard_signal` | class | `message` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:68` |
| `MEM-rxfnsb-standard-signal-module` | `rxfnsb` | `standard_signal` | class | `module` | method | `= .int` | `lib/rxfnsb/rexx/signal.crexx:62` |
| `MEM-rxfnsb-standard-signal-name` | `rxfnsb` | `standard_signal` | class | `name` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:56` |
| `MEM-rxfnsb-standard-signal-payload` | `rxfnsb` | `standard_signal` | class | `payload` | method | `= .object` | `lib/rxfnsb/rexx/signal.crexx:71` |
| `MEM-rxfnsb-standard-signal-source` | `rxfnsb` | `standard_signal` | class | `source` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:83` |
| `MEM-rxfnsb-standard-signalaction-fail` | `rxfnsb` | `standard_signalaction` | class | `fail` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:185` |
| `MEM-rxfnsb-standard-signalaction-kind` | `rxfnsb` | `standard_signalaction` | class | `kind` | method | `= .string` | `lib/rxfnsb/rexx/signal.crexx:189` |
| `MEM-rxfnsb-standard-signalaction-retry` | `rxfnsb` | `standard_signalaction` | class | `retry` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:177` |
| `MEM-rxfnsb-standard-signalaction-skip` | `rxfnsb` | `standard_signalaction` | class | `skip` | factory | `not on declaration line` | `lib/rxfnsb/rexx/signal.crexx:181` |
| `MEM-rxfnsb-stem-default` | `rxfnsb` | `stem` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/stem.crexx:50` |
| `MEM-rxfnsb-stem-get` | `rxfnsb` | `stem` | class | `get` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:86` |
| `MEM-rxfnsb-stem-hash` | `rxfnsb` | `stem` | class | `hash` | method | `= .int` | `lib/rxfnsb/rexx/stem.crexx:63` |
| `MEM-rxfnsb-stem-iterator` | `rxfnsb` | `stem` | class | `iterator` | method | `= .stemIterator` | `lib/rxfnsb/rexx/stem.crexx:211` |
| `MEM-rxfnsb-stem-key` | `rxfnsb` | `stem` | class | `key` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:156` |
| `MEM-rxfnsb-stem-set` | `rxfnsb` | `stem` | class | `set` | method | `= .void` | `lib/rxfnsb/rexx/stem.crexx:108` |
| `MEM-rxfnsb-stem-size` | `rxfnsb` | `stem` | class | `size` | method | `= .int` | `lib/rxfnsb/rexx/stem.crexx:147` |
| `MEM-rxfnsb-stem-snapshotiterator` | `rxfnsb` | `stem` | class | `snapshotIterator` | method | `= .stemIterator` | `lib/rxfnsb/rexx/stem.crexx:222` |
| `MEM-rxfnsb-stem-tails` | `rxfnsb` | `stem` | class | `tails` | method | `= .string[]` | `lib/rxfnsb/rexx/stem.crexx:185` |
| `MEM-rxfnsb-stem-value` | `rxfnsb` | `stem` | class | `value` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:166` |
| `MEM-rxfnsb-stem-valueat` | `rxfnsb` | `stem` | class | `valueAt` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:176` |
| `MEM-rxfnsb-stem-values` | `rxfnsb` | `stem` | class | `values` | method | `= .string[]` | `lib/rxfnsb/rexx/stem.crexx:197` |
| `MEM-rxfnsb-stemiterator-default` | `rxfnsb` | `stemIterator` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/stem.crexx:257` |
| `MEM-rxfnsb-stemiterator-hasnext` | `rxfnsb` | `stemIterator` | class | `hasNext` | method | `= .int` | `lib/rxfnsb/rexx/stem.crexx:279` |
| `MEM-rxfnsb-stemiterator-index` | `rxfnsb` | `stemIterator` | class | `index` | method | `= .int` | `lib/rxfnsb/rexx/stem.crexx:328` |
| `MEM-rxfnsb-stemiterator-islive` | `rxfnsb` | `stemIterator` | class | `isLive` | method | `= .int` | `lib/rxfnsb/rexx/stem.crexx:345` |
| `MEM-rxfnsb-stemiterator-next` | `rxfnsb` | `stemIterator` | class | `next` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:291` |
| `MEM-rxfnsb-stemiterator-reset` | `rxfnsb` | `stemIterator` | class | `reset` | method | `= .void` | `lib/rxfnsb/rexx/stem.crexx:334` |
| `MEM-rxfnsb-stemiterator-value` | `rxfnsb` | `stemIterator` | class | `value` | method | `= .string` | `lib/rxfnsb/rexx/stem.crexx:312` |
| `MEM-rxfnsb-trace-interrupt-raw-address` | `rxfnsb` | `trace_interrupt_raw` | class | `address` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:42` |
| `MEM-rxfnsb-trace-interrupt-raw-code` | `rxfnsb` | `trace_interrupt_raw` | class | `code` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:36` |
| `MEM-rxfnsb-trace-interrupt-raw-module` | `rxfnsb` | `trace_interrupt_raw` | class | `module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:39` |
| `MEM-rxfnsb-trace-interrupt-raw-name` | `rxfnsb` | `trace_interrupt_raw` | class | `name` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:45` |
| `MEM-rxfnsb-tracecontext-default` | `rxfnsb` | `tracecontext` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/trace.crexx:64` |
| `MEM-rxfnsb-tracecontext-address` | `rxfnsb` | `tracecontext` | class | `address` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:95` |
| `MEM-rxfnsb-tracecontext-asm-line` | `rxfnsb` | `tracecontext` | class | `asm_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:122` |
| `MEM-rxfnsb-tracecontext-closest-source-line` | `rxfnsb` | `tracecontext` | class | `closest_source_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:119` |
| `MEM-rxfnsb-tracecontext-column` | `rxfnsb` | `tracecontext` | class | `column` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:107` |
| `MEM-rxfnsb-tracecontext-has-source` | `rxfnsb` | `tracecontext` | class | `has_source` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:116` |
| `MEM-rxfnsb-tracecontext-line` | `rxfnsb` | `tracecontext` | class | `line` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:104` |
| `MEM-rxfnsb-tracecontext-mode` | `rxfnsb` | `tracecontext` | class | `mode` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:98` |
| `MEM-rxfnsb-tracecontext-module` | `rxfnsb` | `tracecontext` | class | `module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:92` |
| `MEM-rxfnsb-tracecontext-procedure` | `rxfnsb` | `tracecontext` | class | `procedure` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:125` |
| `MEM-rxfnsb-tracecontext-signal-code` | `rxfnsb` | `tracecontext` | class | `signal_code` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:86` |
| `MEM-rxfnsb-tracecontext-signal-name` | `rxfnsb` | `tracecontext` | class | `signal_name` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:89` |
| `MEM-rxfnsb-tracecontext-source` | `rxfnsb` | `tracecontext` | class | `source` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:110` |
| `MEM-rxfnsb-tracecontext-source-file` | `rxfnsb` | `tracecontext` | class | `source_file` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:101` |
| `MEM-rxfnsb-tracecontext-source-line` | `rxfnsb` | `tracecontext` | class | `source_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:113` |
| `MEM-rxfnsb-tracecontroller-default` | `rxfnsb` | `tracecontroller` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/trace.crexx:151` |
| `MEM-rxfnsb-tracecontroller-add-included-namespace` | `rxfnsb` | `tracecontroller` | class | `add_included_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:286` |
| `MEM-rxfnsb-tracecontroller-add-suppressed-namespace` | `rxfnsb` | `tracecontroller` | class | `add_suppressed_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:277` |
| `MEM-rxfnsb-tracecontroller-asm-line` | `rxfnsb` | `tracecontroller` | class | `asm_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:391` |
| `MEM-rxfnsb-tracecontroller-capture-result-target` | `rxfnsb` | `tracecontroller` | class | `capture_result_target` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:467` |
| `MEM-rxfnsb-tracecontroller-clear-namespace-cache` | `rxfnsb` | `tracecontroller` | class | `clear_namespace_cache` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:270` |
| `MEM-rxfnsb-tracecontroller-clear-pending-result` | `rxfnsb` | `tracecontroller` | class | `clear_pending_result` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:455` |
| `MEM-rxfnsb-tracecontroller-closest-source-line` | `rxfnsb` | `tracecontroller` | class | `closest_source_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:387` |
| `MEM-rxfnsb-tracecontroller-context` | `rxfnsb` | `tracecontroller` | class | `context` | method | `= .tracecontext` | `lib/rxfnsb/rexx/trace.crexx:345` |
| `MEM-rxfnsb-tracecontroller-context-from-interrupt` | `rxfnsb` | `tracecontroller` | class | `context_from_interrupt` | method | `= .tracecontext` | `lib/rxfnsb/rexx/trace.crexx:350` |
| `MEM-rxfnsb-tracecontroller-disable-breakpoints` | `rxfnsb` | `tracecontroller` | class | `disable_breakpoints` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:341` |
| `MEM-rxfnsb-tracecontroller-enable-breakpoints` | `rxfnsb` | `tracecontroller` | class | `enable_breakpoints` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:337` |
| `MEM-rxfnsb-tracecontroller-exact-source-line` | `rxfnsb` | `tracecontroller` | class | `exact_source_line` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:378` |
| `MEM-rxfnsb-tracecontroller-find-proc` | `rxfnsb` | `tracecontroller` | class | `find_proc` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:451` |
| `MEM-rxfnsb-tracecontroller-first-client-module` | `rxfnsb` | `tracecontroller` | class | `first_client_module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:194` |
| `MEM-rxfnsb-tracecontroller-include-runtime` | `rxfnsb` | `tracecontroller` | class | `include_runtime` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:212` |
| `MEM-rxfnsb-tracecontroller-latest-module-only` | `rxfnsb` | `tracecontroller` | class | `latest_module_only` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:203` |
| `MEM-rxfnsb-tracecontroller-load-library` | `rxfnsb` | `tracecontroller` | class | `load_library` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:414` |
| `MEM-rxfnsb-tracecontroller-load-module` | `rxfnsb` | `tracecontroller` | class | `load_module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:408` |
| `MEM-rxfnsb-tracecontroller-loaded-module-count` | `rxfnsb` | `tracecontroller` | class | `loaded_module_count` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:399` |
| `MEM-rxfnsb-tracecontroller-loaded-proc-count` | `rxfnsb` | `tracecontroller` | class | `loaded_proc_count` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:418` |
| `MEM-rxfnsb-tracecontroller-loaded-proc-id` | `rxfnsb` | `tracecontroller` | class | `loaded_proc_id` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:438` |
| `MEM-rxfnsb-tracecontroller-loaded-proc-name` | `rxfnsb` | `tracecontroller` | class | `loaded_proc_name` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:425` |
| `MEM-rxfnsb-tracecontroller-mode` | `rxfnsb` | `tracecontroller` | class | `mode` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:173` |
| `MEM-rxfnsb-tracecontroller-module-name` | `rxfnsb` | `tracecontroller` | class | `module_name` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:404` |
| `MEM-rxfnsb-tracecontroller-namespace-is-suppressed` | `rxfnsb` | `tracecontroller` | class | `namespace_is_suppressed` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:255` |
| `MEM-rxfnsb-tracecontroller-namespace-list-contains` | `rxfnsb` | `tracecontroller` | class | `namespace_list_contains` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:323` |
| `MEM-rxfnsb-tracecontroller-namespace-list-matches` | `rxfnsb` | `tracecontroller` | class | `namespace_list_matches` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:330` |
| `MEM-rxfnsb-tracecontroller-pending-const` | `rxfnsb` | `tracecontroller` | class | `pending_const` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:520` |
| `MEM-rxfnsb-tracecontroller-pending-has-const` | `rxfnsb` | `tracecontroller` | class | `pending_has_const` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:523` |
| `MEM-rxfnsb-tracecontroller-pending-module` | `rxfnsb` | `tracecontroller` | class | `pending_module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:508` |
| `MEM-rxfnsb-tracecontroller-pending-prefix` | `rxfnsb` | `tracecontroller` | class | `pending_prefix` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:511` |
| `MEM-rxfnsb-tracecontroller-pending-register` | `rxfnsb` | `tracecontroller` | class | `pending_register` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:517` |
| `MEM-rxfnsb-tracecontroller-pending-result` | `rxfnsb` | `tracecontroller` | class | `pending_result` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:505` |
| `MEM-rxfnsb-tracecontroller-pending-type` | `rxfnsb` | `tracecontroller` | class | `pending_type` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:514` |
| `MEM-rxfnsb-tracecontroller-procedure-name` | `rxfnsb` | `tracecontroller` | class | `procedure_name` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:395` |
| `MEM-rxfnsb-tracecontroller-remove-included-namespace` | `rxfnsb` | `tracecontroller` | class | `remove_included_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:309` |
| `MEM-rxfnsb-tracecontroller-remove-suppressed-namespace` | `rxfnsb` | `tracecontroller` | class | `remove_suppressed_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:295` |
| `MEM-rxfnsb-tracecontroller-reset-namespace-filters` | `rxfnsb` | `tracecontroller` | class | `reset_namespace_filters` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:215` |
| `MEM-rxfnsb-tracecontroller-set-first-client-module` | `rxfnsb` | `tracecontroller` | class | `set_first_client_module` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:188` |
| `MEM-rxfnsb-tracecontroller-set-include-runtime` | `rxfnsb` | `tracecontroller` | class | `set_include_runtime` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:206` |
| `MEM-rxfnsb-tracecontroller-set-latest-module-only` | `rxfnsb` | `tracecontroller` | class | `set_latest_module_only` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:197` |
| `MEM-rxfnsb-tracecontroller-set-mode` | `rxfnsb` | `tracecontroller` | class | `set_mode` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:176` |
| `MEM-rxfnsb-tracecontroller-should-trace` | `rxfnsb` | `tracecontroller` | class | `should_trace` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:354` |
| `MEM-rxfnsb-tracecontroller-should-trace-module` | `rxfnsb` | `tracecontroller` | class | `should_trace_module` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:365` |
| `MEM-rxfnsb-tracecontroller-supplied-result` | `rxfnsb` | `tracecontroller` | class | `supplied_result` | method | `= .int` | `lib/rxfnsb/rexx/trace.crexx:532` |
| `MEM-rxfnsb-tracecontroller-supplied-value` | `rxfnsb` | `tracecontroller` | class | `supplied_value` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:535` |
| `MEM-rxfnsb-tracecontroller-supply-parent-value` | `rxfnsb` | `tracecontroller` | class | `supply_parent_value` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:526` |
| `MEM-rxfnsb-tracecontroller-suppress-namespace` | `rxfnsb` | `tracecontroller` | class | `suppress_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:237` |
| `MEM-rxfnsb-tracecontroller-toggle-mode` | `rxfnsb` | `tracecontroller` | class | `toggle_mode` | method | `= .string` | `lib/rxfnsb/rexx/trace.crexx:183` |
| `MEM-rxfnsb-tracecontroller-unsuppress-namespace` | `rxfnsb` | `tracecontroller` | class | `unsuppress_namespace` | method | `= .void` | `lib/rxfnsb/rexx/trace.crexx:246` |
| `MEM-rxhttp-http-default` | `rxhttp` | `http` | class | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/rxhttp.crexx:81` |
| `MEM-rxhttp-http-body-start` | `rxhttp` | `http` | class | `_body_start` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:297` |
| `MEM-rxhttp-http-byte-length` | `rxhttp` | `http` | class | `_byte_length` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:432` |
| `MEM-rxhttp-http-chunked-complete` | `rxhttp` | `http` | class | `_chunked_complete` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:399` |
| `MEM-rxhttp-http-content-length` | `rxhttp` | `http` | class | `_content_length` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:307` |
| `MEM-rxhttp-http-decode-chunked-body` | `rxhttp` | `http` | class | `_decode_chunked_body` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:347` |
| `MEM-rxhttp-http-has-complete-body` | `rxhttp` | `http` | class | `_has_complete_body` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:283` |
| `MEM-rxhttp-http-header-value` | `rxhttp` | `http` | class | `_header_value` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:320` |
| `MEM-rxhttp-http-hex-digit-value` | `rxhttp` | `http` | class | `_hex_digit_value` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:473` |
| `MEM-rxhttp-http-hex-to-int` | `rxhttp` | `http` | class | `_hex_to_int` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:462` |
| `MEM-rxhttp-http-is-chunked` | `rxhttp` | `http` | class | `_is_chunked` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:314` |
| `MEM-rxhttp-http-parse-status` | `rxhttp` | `http` | class | `_parse_status` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:479` |
| `MEM-rxhttp-http-read-response` | `rxhttp` | `http` | class | `_read_response` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:254` |
| `MEM-rxhttp-http-send-212` | `rxhttp` | `http` | class | `_send` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:212` |
| `MEM-rxhttp-http-set-error` | `rxhttp` | `http` | class | `_set_error` | method | `not on declaration line` | `lib/rxfnsb/rexx/rxhttp.crexx:489` |
| `MEM-rxhttp-http-take-bytes` | `rxhttp` | `http` | class | `_take_bytes` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:444` |
| `MEM-rxhttp-http-buildrequest` | `rxhttp` | `http` | class | `buildRequest` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:118` |
| `MEM-rxhttp-http-buildrequestwithheaders` | `rxhttp` | `http` | class | `buildRequestWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:123` |
| `MEM-rxhttp-http-error` | `rxhttp` | `http` | class | `error` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:203` |
| `MEM-rxhttp-http-extractbody` | `rxhttp` | `http` | class | `extractBody` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:144` |
| `MEM-rxhttp-http-header` | `rxhttp` | `http` | class | `header` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:193` |
| `MEM-rxhttp-http-httpstatus` | `rxhttp` | `http` | class | `httpStatus` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:200` |
| `MEM-rxhttp-http-lastbody` | `rxhttp` | `http` | class | `lastBody` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:209` |
| `MEM-rxhttp-http-lasthttp` | `rxhttp` | `http` | class | `lastHttp` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:206` |
| `MEM-rxhttp-http-post` | `rxhttp` | `http` | class | `post` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:109` |
| `MEM-rxhttp-http-postwithheaders` | `rxhttp` | `http` | class | `postWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:114` |
| `MEM-rxhttp-http-send-95` | `rxhttp` | `http` | class | `send` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:95` |
| `MEM-rxhttp-http-sendwithheaders` | `rxhttp` | `http` | class | `sendWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:100` |
| `MEM-rxhttp-http-status` | `rxhttp` | `http` | class | `status` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:197` |
| `MEM-rxhttp-httpclient-default` | `rxhttp` | `httpclient` | interface | `*` | factory | `not on declaration line` | `lib/rxfnsb/rexx/rxhttp.crexx:32` |
| `MEM-rxhttp-httpclient-buildrequest` | `rxhttp` | `httpclient` | interface | `buildRequest` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:47` |
| `MEM-rxhttp-httpclient-buildrequestwithheaders` | `rxhttp` | `httpclient` | interface | `buildRequestWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:50` |
| `MEM-rxhttp-httpclient-error` | `rxhttp` | `httpclient` | interface | `error` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:63` |
| `MEM-rxhttp-httpclient-extractbody` | `rxhttp` | `httpclient` | interface | `extractBody` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:53` |
| `MEM-rxhttp-httpclient-header` | `rxhttp` | `httpclient` | interface | `header` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:56` |
| `MEM-rxhttp-httpclient-httpstatus` | `rxhttp` | `httpclient` | interface | `httpStatus` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:61` |
| `MEM-rxhttp-httpclient-lastbody` | `rxhttp` | `httpclient` | interface | `lastBody` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:67` |
| `MEM-rxhttp-httpclient-lasthttp` | `rxhttp` | `httpclient` | interface | `lastHttp` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:65` |
| `MEM-rxhttp-httpclient-post` | `rxhttp` | `httpclient` | interface | `post` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:41` |
| `MEM-rxhttp-httpclient-postwithheaders` | `rxhttp` | `httpclient` | interface | `postWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:44` |
| `MEM-rxhttp-httpclient-send` | `rxhttp` | `httpclient` | interface | `send` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:35` |
| `MEM-rxhttp-httpclient-sendwithheaders` | `rxhttp` | `httpclient` | interface | `sendWithHeaders` | method | `= .string` | `lib/rxfnsb/rexx/rxhttp.crexx:38` |
| `MEM-rxhttp-httpclient-status` | `rxhttp` | `httpclient` | interface | `status` | method | `= .int` | `lib/rxfnsb/rexx/rxhttp.crexx:59` |

Discovered rows: **312**.
