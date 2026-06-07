CC      = clang
CFLAGS  = -O2 -Wall -mmacosx-version-min=12.0
LDFLAGS = -framework IOKit -framework CoreFoundation
TARGET  = axefx-usb-reset
ARM64   = $(TARGET)-arm64
X86_64  = $(TARGET)-x86_64

$(TARGET): $(TARGET).c
	$(CC) $(CFLAGS) -target arm64-apple-macos12.0  -o $(ARM64)  $< $(LDFLAGS)
	$(CC) $(CFLAGS) -target x86_64-apple-macos12.0 -o $(X86_64) $< $(LDFLAGS)
	lipo -create -output $@ $(ARM64) $(X86_64)
	rm -f $(ARM64) $(X86_64)

install: $(TARGET)
	install -m 755 $(TARGET) ~/.local/bin/$(TARGET)

clean:
	rm -f $(TARGET) $(ARM64) $(X86_64)

.PHONY: install clean
