include makefile.common

# This may change
INCLUDES += -I../Include/DirectXMath
TARGETDIR = ../Lib/x64/$(CONFIG)
TARGET = $(TARGETDIR)/libabsmath.a
SOURDEDIR = ../absmath/absmath
OBJDIR = obj/x64/$(CONFIG)/absmath
PCH = $(SOURDEDIR)/stdafx.h
CXXFLAGS += -Werror -Wno-class-memaccess -Wno-maybe-uninitialized
# End changes

SRC_FILES = $(filter-out $(SOURDEDIR)/stdafx.cpp, $(wildcard $(SOURDEDIR)/*.cpp))

CXXFLAGS += $(DEFINES) $(INCLUDES)

OBJ_FILES := $(patsubst $(SOURDEDIR)/%.cpp, $(OBJDIR)/%.o, $(SRC_FILES))
#$(info $(OBJ_FILES))
GCH = $(PCH).gch

all: $(TARGET)

$(TARGET): $(GCH) $(OBJ_FILES)
	@$(MKDIR_P) $(@D)
	$(LINKCMD_LIB) $(OBJ_FILES)

$(OBJDIR)/%.o: $(SOURDEDIR)/%.cpp
	@$(MKDIR_P) $(@D)
	$(PRE_CXX) $(CXX) $(CXXFLAGS) -MMD -c $< -o $@

# PCH
$(GCH): $(PCH)
	$(CXX) -x c++-header $(CXXFLAGS) -c $< -o $@

-include $(OBJ_FILES:.o=.d)

.PHONY: clean
clean:
	rm -f $(GCH) $(OBJ_FILES) $(TARGET) $(OBJDIR)/*.d
