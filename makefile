NAME = exe

CXX_SRC =\
	main.cpp \
	utils.cpp \
	version.cpp

C_SRC =\
	timestamp.c

CXX = g++
LL = g++
CC = gcc
OPTIMIZATION_FLAGS =
CXX_FLAGS = -std=c++17 -Wall -Wextra -MMD -MF $(patsubst %.o,%.d,$@)
CXX_FLAGS += -DPROGRAM_VERSION="$(shell git describe)"
CC_FLAGS = -Wall -Wextra
LD_FLAGS =
DBG_FLAGS = -ggdb -DDEBUG=8
REL_FLAGS = -O2
INCLUDE = -Iinclude
LIBS := -lstdc++
LIBS += $(shell pkg-config --libs gl glew glfw3)
LIBS += -Llib -lktx
SRC_DIR = src
OBJ_DIR = obj

_OBJ := $(CXX_SRC:%.cpp=%.o)
_OBJ += $(C_SRC:%.c=%.o)
OBJ = $(_OBJ:%=$(OBJ_DIR)/%)

DEPS := $(OBJ:%.o=%.d)

TAGS_FLAGS := --fields=* --extras=* --extras-c++=* -R
TAGS_FLAGS += $(SRC_DIR) /usr/include/{GL,GLFW,glm}/* ./include/*

all: CXX_FLAGS += $(DBG_FLAGS)
all: CC_FLAGS += $(DBG_FLAGS)
all: build

release: CXX_FLAGS += $(REL_FLAGS)
release: CC_FLAGS += $(REL_FLAGS)
release: build
	@strip $(NAME)

.PHONY: build
build: $(OBJ_DIR) $(NAME)

$(NAME): $(OBJ)
	@echo "LL $@"
	@$(LL) -o $@ $(OBJ) $(LIBS)

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.cpp makefile
	@echo "CXX $< -> $@"
	@$(CXX) $(INCLUDE) $(CXX_FLAGS) -c -o $@ $<

$(OBJ_DIR)/%.o: $(SRC_DIR)/%.c makefile
	@echo "CC $< -> $@"
	@$(CC) $(INCLUDE) $(CC_FLAGS) -c -o $@ $<

$(OBJ_DIR):
	mkdir -p $@

.PHONY: clean
clean:
	@rm -vrf $(OBJ_DIR)
	@rm -vf $(NAME)

.PHONY: ctags
ctags:
	ctags $(TAGS_FLAGS)

.PHONY: ctags_e
ctags_e:
	ctags -e $(TAGS_FLAGS)

-include $(DEPS)
