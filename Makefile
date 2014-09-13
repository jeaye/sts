# sts - simple terminal scroller

include config.mk

OUT = bin/
SRC = src/main.cpp
OBJ = ${SRC:.cpp=.cpp.o}
TARGET = ${OUT}sts

all: ${TARGET}

options:
	echo "CXXFLAGS = ${CXXFLAGS}"
	echo "LDFLAGS  = ${LDFLAGS}"
	echo "CXX      = ${CXX}"
	echo

setup:
	mkdir -p ${OUT}

%.cpp.o: %.cpp setup
	echo "compiling $<"
	${CXX} -o $@ -c ${CXXFLAGS} $<

${TARGET}: options ${OBJ}
	echo "linking $@"
	${CXX} -o $@ ${OBJ} ${LDFLAGS}

clean:
	echo "cleaning"
	rm -f ${TARGET} ${OBJ} sts-${VERSION}.tar.gz

install: ${TARGET}
	echo "installing executable file to ${DESTDIR}${PREFIX}/bin"
	mkdir -p ${DESTDIR}${PREFIX}/bin
	cp -f ${TARGET} ${DESTDIR}${PREFIX}/bin
	chmod 755 ${DESTDIR}${PREFIX}/bin/sts
	echo "installing manual page to ${DESTDIR}${MANPREFIX}/man1"
	mkdir -p ${DESTDIR}${MANPREFIX}/man1
	sed "s/VERSION/${VERSION}/g" < sts.1 > ${DESTDIR}${MANPREFIX}/man1/sts.1
	chmod 644 ${DESTDIR}${MANPREFIX}/man1/sts.1

uninstall:
	echo "removing executable file from ${DESTDIR}${PREFIX}/bin"
	rm -f ${DESTDIR}${PREFIX}/bin/sts
	echo "removing manual page from ${DESTDIR}${MANPREFIX}/man1"
	rm -f ${DESTDIR}${MANPREFIX}/man1/sts.1

# for debugging purposes only
shell:
	st

run:
	st -e ${TARGET}

.SILENT:
.PHONY: all options setup clean dist install uninstall shell run
