GPP_PARAMS = -D_SHARED_USER -O0 -ffreestanding -Wall -Wextra -fno-exceptions -fno-rtti -fno-use-cxa-atexit -ggdb -Wno-unused-variable -Wno-write-strings -Wno-unused-parameter -fstack-protector-all

SHARED_CPP_FILES := $(shell find ../shared -type f -name '*.cpp')
LIBS_CPP_FILES := $(shell find include -type f -name '*.cpp')
objects := $(patsubst %.cpp,%.o,$(filter %.cpp,$(LIBS_CPP_FILES)))
shared_objects := $(patsubst ../shared/%.cpp,./include/%.o,$(SHARED_CPP_FILES))

SRC_DIRS := $(wildcard src/*/)

clean:
	find . -name "*.o" -type f -delete
	find ./include -name "*.o" -type f -delete
	find ../iso/boot/software/ -name "*.bin" -type f -delete

./include/%.o: ../shared/%.cpp
	i686-elf-g++ $(GPP_PARAMS) -o $@ -c $<

%.o: %.cpp
	i686-elf-g++ $(GPP_PARAMS) -o $@ -c $<

libs: $(objects) $(shared_objects)

all: $(SRC_DIRS)

.PHONY: $(SRC_DIRS)
$(SRC_DIRS): libs 
	$(MAKE) -C $@ build install --always-make