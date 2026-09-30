# About native libraries

CREXX distribution packages include selected native providers with both dynamic
VM and native-static delivery. In particular, the [rxsqlite provider](rxsqlite.md)
includes its pinned SQLite implementation, while `rxsqlite_address` is an
optional Level G façade over it. Other integrations can still require a
separately installed product library. ODBC and GTK, for example, depend on
native facilities supplied for the target platform.

When these libraries are not delivered with the language product itself, there is a distinct possibility of API skew between the cRexx part of the library and the products themselves. This means that a newer version of the external product library has impact on the functioning of the cRexx library API. Insulation between the upgrade paths is always a prudent strategy here, for example by using a virtual machine or a container image. This approach is not necessary when using the cRexx API's proper, for which the contract is guaranteed.

Usage of the native implementations of these parts of the library can offer great benefits to the cRexx application developer, and enables cRexx to participate in a multi-language project by connecting to the same infrastructure. With installation and maintenance of third party libraries there is a larger investment required than with the core library; for this reason, every update of the library documentation will indicate with which release of the external interface libraries the verification has taken place.

Some of the available (procedural or object oriented) libraries are included in the main distributions and other might require a cRexx build from source with special options. The documentation indicates when this is needed. The development team is aware of the fact that this is an even deeper level of investment; on the other hand, there are clear instructions for building the system in the *Programming Guide* and there is a certain satisfaction that goes with building one's own tools.

The different repository systems like *brew*, *apt* and *chocolatey* can deliver assistance in installing external libraries in the required releases.
