TARGET = example

CXX = g++
INCLUDES =  -I../src

CXXFLAGS = -MMD -O3 -Wall $(INCLUDES)
OBJ_DIR ?= obj
LFLAGS = 
SHARED_LIBS = -lgumbo

SOURCES := main.cpp \
	../src/Document.cpp \
	../src/Node.cpp \
	../src/Object.cpp \
	../src/Parser.cpp \
	../src/QueryUtil.cpp \
	../src/Selection.cpp \
	../src/Selector.cpp

OBJECTS := $(SOURCES:.cpp=.o)
OBJECTS := $(subst ../,,$(OBJECTS))
OBJECTS := $(addprefix $(OBJ_DIR)/, $(OBJECTS))

DEPS = $(OBJECTS:.o=.d)

all: $(TARGET) Makefile

$(TARGET): $(OBJECTS) Makefile
	        $(CXX) $(CXXFLAGS) $(OBJECTS) -o $@ $(LFLAGS) $(SHARED_LIBS)

$(OBJ_DIR)/%.o: %.cpp Makefile
	        mkdir -p $(@D)
			$(CXX) $(CXXFLAGS) -c $< -o $@

$(OBJ_DIR)/%.o: ../%.cpp Makefile
	        mkdir -p $(@D)
			$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	        rm -rf obj

-include $(DEPS)
