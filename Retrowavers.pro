APP_NAME = Retrowavers

CONFIG += qt warn_on cascades10
LIBS   += -lbbdata -lbbnetwork -lbbmultimedia -lbb -lbbplatform -lbbsystem -lbbdevice

QT += network

# Build environment, the same axis the other projects use: bb-shared/tools/build-bar.sh
# exports BBT_ENV (development unless -e production). Momentics knows nothing about it,
# so a plain IDE build is development - which is what you want while developing.
BBT_ENV = $$(BBT_ENV)
isEmpty(BBT_ENV): BBT_ENV = development
# contains(), not equals(): qmake re-reads this file for the translations subproject
# and BBT_ENV can arrive as a LIST ("development production") - an equals() check then
# quietly falls through to the development branch on some of the passes.
contains(BBT_ENV, production) {
    DEFINES += RW_PRODUCTION
    message("Retrowavers: production build - startup trace and file log compiled out")
} else {
    message("Retrowavers: development build ($$BBT_ENV) - startup trace enabled")
}

# Fail the BUILD on an unresolved symbol instead of the device. A Cascades app is
# linked as a shared object, and -shared does not check that every symbol resolves,
# so a method that is declared and called but never defined links cleanly and then
# kills the process at load time - before main(), so not even a log line survives.
# That cost us an afternoon once; the linker can catch it in a second.
QMAKE_LFLAGS += -Wl,--no-undefined

include(config.pri)
