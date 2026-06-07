CC      = clang
CFLAGS  = -O2 -Wall
LDFLAGS = -framework IOKit -framework CoreFoundation
TARGET  = axefx-usb-reset

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -o $@ $< $(LDFLAGS)

install: $(TARGET)
	install -m 755 $(TARGET) ~/.local/bin/$(TARGET)

clean:
	rm -f $(TARGET)

.PHONY: install clean
