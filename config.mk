VERSION = 0.1alpha

# customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS += -Iinclude
LIBS += 

# flags
CPPFLAGS += -DVERSION=\"${VERSION}\"
CXXFLAGS += -std=c++14 -Wall -Wextra -pedantic -O3 ${INCS} ${CPPFLAGS}
LDFLAGS += ${LIBS}

# compiler and linker
CXX ?= c++
