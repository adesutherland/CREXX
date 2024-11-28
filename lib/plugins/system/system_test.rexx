/* SysInfo Plugin Test */
options levelb
import system

say "getEnv(PATH) "getEnv("PATH")
say "getDIR       "getdir()
say "setDIR(C:/temp) "setdir("c:/Temp")
say "getDIR       "getdir()
say "CreateDIR(c:/Temp/crexx123) "createdir("c:/Temp/crexx123")
say "CreateDIR(C:/Temp/crexx456) "createdir("c:/Temp/crexx456")
say "RemoveDIR(c:/Temp/crexx123) "removedir("c:/Temp/crexx123")
say "RemoveDIR(c:/Temp/crexx123) "removedir("c:/Temp/crexx123")


