VERSION = 0.1

# customize below to fit your system

# paths
PREFIX = /usr/local
MANPREFIX = ${PREFIX}/share/man

# includes and libs
INCS += -Iinclude
LIBS += 

# flags
CPPFLAGS += -DVERSION=\"${VERSION}\"
CXXFLAGS += -std=c++11 -Wall -Wextra -Werror -pedantic -O3 ${INCS} ${CPPFLAGS}
LDFLAGS += ${LIBS}

# compiler and linker
CXX ?= c++
