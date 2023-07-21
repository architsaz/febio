programs= febmkr logmkr inputmkr
all: $(programs)

BUILD_DIR := ./build
SRC_DIRS_pro1 := ./src/febmkr
SRC_DIRS_pro2 := ./src/logmkr
SRC_DIRS_pro3 := ./src/inputmkr
BIN_DIR := ./build/bin

# Find all the C and C++ files we want to compile
# Note the single quotes around the * expressions. The shell will incorrectly expand these otherwise, but we want to send the * directly to the find command.
SRCS_pro1 := $(shell find $(SRC_DIRS_pro1) -name '*.cpp' -or -name '*.c' -or -name '*.s')
SRCS_pro2 := $(shell find $(SRC_DIRS_pro2) -name '*.cpp' -or -name '*.c' -or -name '*.s')
SRCS_pro3 := $(shell find $(SRC_DIRS_pro3) -name '*.cpp' -or -name '*.c' -or -name '*.s')

# Prepends BUILD_DIR and appends .o to every src file
# As an example, ./your_dir/hello.cpp turns into ./build/./your_dir/hello.cpp.o
OBJS_pro1 := $(SRCS_pro1:%=$(BUILD_DIR)/%.o)
OBJS_pro2 := $(SRCS_pro2:%=$(BUILD_DIR)/%.o)
OBJS_pro3 := $(SRCS_pro3:%=$(BUILD_DIR)/%.o)

#Find all the included files we need to compile



CFLAGS := -lm

febmkr : $(OBJS_pro1)
	$(CXX) $(OBJS_pro1) -o $(BIN_DIR)/$@ $(LDFLAGS)

logmkr : $(OBJS_pro2)
	$(CXX) $(OBJS_pro2) -o $(BIN_DIR)/$@ $(LDFLAGS)

inputmkr : $(OBJS_pro3)
	$(CXX) $(OBJS_pro3) -o $(BIN_DIR)/$@ $(LDFLAGS)

# Build step for C source
$(BUILD_DIR)/%.c.o: %.c
	echo $(addprefix -I,$(dir $^))
	mkdir -p $(dir $@)
	mkdir -p $(BIN_DIR)
	$(CC) $(CPPFLAGS) $(CFLAGS) -c $< -o $@

# Build step for C++ source
$(BUILD_DIR)/%.cpp.o: %.cpp
	mkdir -p $(dir $@)
	mkdir -p $(BIN_DIR)
	$(CXX) $(CPPFLAGS) $(CXXFLAGS) -c $< -o $@

.PHONY : clean
clean:
	rm -r ./build
