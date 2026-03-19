.PHONY : clean all

# Compiler to be used
CC=gcc
CXX=g++

# Standard flags to be used
CXXFLAGS = -fPIC -std=gnu++11 -pthread -Wall
CFLAGS = -fPIC -pthread -Wall

# Reading the project specific configurations
# PROGRAMNAME =
# DEFINES =
# INCLUDES =
# CPPFILES =
# CFILES = 
# STATLIBS =
# DYNLIBS =
# DYNLIBDIRS =
# CXXFLAGS =
# CFLAGS =
# LNKFLAGS =

include sources

OUTPUT := ./bin

SOURCE_DIRS := $(dir $(CPPFILES))
SOURCE_DIRS += $(dir $(CFILES))
VPATH = $(sort $(SOURCE_DIRS))

CPP_FILENAMES := $(notdir $(CPPFILES))
C_FILENAMES := $(notdir $(CFILES))

OBJ_FILES := $(patsubst %.cpp, $(OUTPUT)/%.o, $(CPP_FILENAMES) )
OBJ_FILES += $(patsubst %.c, $(OUTPUT)/%.o, $(C_FILENAMES) )

all : $(OUTPUT)/$(PROGRAMNAME)

$(OUTPUT)/$(PROGRAMNAME) : $(OBJ_FILES)
	g++ $(LNKFLAGS) $(DYNLIBDIRS) -o $@ $^ $(STATLIBS) $(DYNLIBS)

$(OUTPUT)/%.o : %.cpp
	$(CXX) $(CXXFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

$(OUTPUT)/%.o : %.c
	$(CC) $(CFLAGS) $(DEFINES) $(INCLUDES) -c $< -o $@

clean:
ifeq ($(OS),Windows_NT)    # is always Windows_NT on XP, 2000, 7, Vista, 10...
	del /Q "$(OUTPUT)\*"
else
	rm -fr $(OUTPUT)
endif
