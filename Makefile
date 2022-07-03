PWD := $(shell pwd)

SDIR := $(PWD)/src
IDIR := $(PWD)/include
ODIR := $(PWD)/bin

SFILES := $(wildcard $(SDIR)/*.cpp)
OFILES := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SFILES))

OUT := systemviewer
GIT_VERSION := "$(shell git describe --abbrev=4 --dirty --always --tags)"

CC := g++
CFLAGS := -std=c++20 -Wall -Wextra -MP -MD -I$(IDIR) -g -Wno-unused
CFLAGS += -DVERSION=\"$(GIT_VERSION)\"

-include $(SDIR/:.cpp=.d)

all: ${OFILES}
	$(CC) $(CFLAGS) ${OFILES} -o $(OUT)

clean:
	rm ${OFILES}

$(ODIR)/%.o : $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

