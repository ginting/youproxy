CC = /opt/android-ndk-r8b/toolchains/arm-linux-androideabi-4.6/prebuilt/darwin-x86/bin/arm-linux-androideabi-g++
SDK = /opt/android-ndk-r8b/platforms/android-9/arch-arm
CFLAGS = --sysroot $(SDK) -I../../src -D__LINUX__ 
CFLAGS += -isystem/opt/android-ndk-r8b/sources/cxx-stl/gnu-libstdc++/4.6/include
CFLAGS += -isystem/opt/android-ndk-r8b/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi/include
LDFLAGS = $(CFLAGS)
LDFLAGS += -L/opt/android-ndk-r8b/sources/cxx-stl/gnu-libstdc++/4.6/libs/armeabi -lgnustl_static
OBJS = main.o setproxy.o $(patsubst %.cpp,%.o, $(wildcard ../../src/*.cpp))

all: youagent
	echo 'Done.'

youagent: $(OBJS)
	$(CC) -o $@ $(OBJS) $(LDFLAGS)


%.o:%.cpp
	$(CC) -c $(CFLAGS) -o $@ $<

.PHONY: clean
clean:
	rm -rf $(OBJS)

