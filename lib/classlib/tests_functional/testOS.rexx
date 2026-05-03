options levelb comments_dash
import system_os

x = .os()
say x.uptime()
say x.user()
lp = x.loadpath() 
say lp
say x.osname()
say x.getHost()
say 'waiting one second:'
say x.wait(1000)
say x.beep()
say x.rxbinModules(lp'/library.rxbin')
say x.rxbinModules(lp'/classlib.rxbin')
say x.getEnv("HOME")

