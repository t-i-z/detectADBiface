CC = gcc
CFLAGS += -W -Wall -Wextra -Werror
EXE = detectADBiface

all: $(EXE)

%: %.c
	$(CC) $(CFLAGS) $< -o $@ -l:libusb-1.0.a -ludev

clean:
	$(RM) *.o $(EXE)
