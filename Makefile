PWD := $(shell pwd)

SDIR := $(PWD)/src
IDIR := $(PWD)/include
ODIR := $(PWD)/bin

SFILES := $(wildcard $(SDIR)/*.cpp)
OFILES := $(patsubst $(SDIR)/%.cpp,$(ODIR)/%.o,$(SFILES))

OUT := spacelike

CC := g++
CFLAGS := -std=c++20 -Wall -Wextra -MP -MD -I$(IDIR) -g -Wno-unused

-include $(SDIR/:.cpp=.d)

all: ${OFILES}
	$(CC) $(CFLAGS) ${OFILES} -o $(OUT)

clean:
	rm ${OFILES}

$(ODIR)/%.o : $(SDIR)/%.cpp
	$(CC) $(CFLAGS) -c $< -o $@

