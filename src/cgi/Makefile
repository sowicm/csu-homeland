CPPFLAGS=-c -Wall -Ofast -std=c++11 -fexec-charset=UTF-8
CC=g++
CommonObjs=cgic.o md5.o scgi.o vars.o dbconfig.o
VPATH=common
LIBS=-lpthread -L/usr/lib/mysql/ -lmysqlclient -lz -ldl -static
# -L. -lstdc++
# -static-libstdc++

all: index.cgi

index.cgi: $(CommonObjs) main.o
	${CC} -s $(CommonObjs) main.o -o ../bin/index.cgi $(LIBS)

main.o: index/main.cpp cgic.h scgi.h md5.h
	${CC} ${CPPFLAGS} index/main.cpp

cgic.o: cgic.cpp cgic.h
	${CC} ${CPPFLAGS} common/cgic.cpp

md5.o: md5.cpp md5.h 
	${CC} ${CPPFLAGS} common/md5.cpp

vars.o: vars.cpp vars.h
	${CC} ${CPPFLAGS} common/vars.cpp

scgi.o: scgi.cpp scgi.h cgic.h
	${CC} ${CPPFLAGS} common/scgi.cpp

dbconfig.o: dbconfig.cpp dbconfig.h
	${CC} ${CPPFLAGS} common/dbconfig.cpp

# for test begin
helloworld.cgi: cgic.o scgi.o vars.o helloworld.o
	${CC} -s cgic.o scgi.o vars.o helloworld.o -o $@ $(LIBS)

helloworld.o: helloworld/helloworld.cpp
	${CC} ${CPPFLAGS} $<

sendmail.cgi: sendmail.o
	${CC} -s $< -o ../bin/sendmail.cgi $(LIBS)

sendmail.o: test/sendmail.cpp
	${CC} ${CPPFLAGS} $<

works.cgi: works.o
	${CC} -s $< -o $@ $(LIBS)

works.o: works.cpp
	${CC} ${CPPFLAGS} $<
# for test end

clean:
	find . -name "*.o"  | xargs rm -f
	-rm -f index.cgi helloworld.cgi

.PHONY: all clean
