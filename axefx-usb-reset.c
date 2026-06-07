#include <stdio.h>
#include <string.h>
#include <signal.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>
#include <libproc.h>

#define VENDOR_ID  9318   /* 0x2466 Fractal Audio */
#define PRODUCT_ID 32784  /* 0x8010 Axe-Fx III   */

static int usb_reenumerate(void) {
    CFMutableDictionaryRef match = IOServiceMatching(kIOUSBDeviceClassName);
    SInt32 v = VENDOR_ID, p = PRODUCT_ID;
    CFDictionarySetValue(match, CFSTR(kUSBVendorID),  CFNumberCreate(NULL, kCFNumberSInt32Type, &v));
    CFDictionarySetValue(match, CFSTR(kUSBProductID), CFNumberCreate(NULL, kCFNumberSInt32Type, &p));

    io_iterator_t iter = 0;
    kern_return_t kr = IOServiceGetMatchingServices(kIOMainPortDefault, match, &iter);
    if (kr != KERN_SUCCESS) { fprintf(stderr, "IOServiceGetMatchingServices failed: %d\n", kr); return 1; }

    io_service_t svc = IOIteratorNext(iter);
    IOObjectRelease(iter);
    if (!svc) { fprintf(stderr, "Axe-Fx III not found\n"); return 1; }

    IOCFPlugInInterface **plug = NULL;
    SInt32 score = 0;
    kr = IOCreatePlugInInterfaceForService(svc, kIOUSBDeviceUserClientTypeID,
                                          kIOCFPlugInInterfaceID, &plug, &score);
    IOObjectRelease(svc);
    if (kr != KERN_SUCCESS || !plug) { fprintf(stderr, "Plugin create failed: %d\n", kr); return 1; }

    IOUSBDeviceInterface **dev = NULL;
    HRESULT hr = (*plug)->QueryInterface(plug, CFUUIDGetUUIDBytes(kIOUSBDeviceInterfaceID),
                                         (LPVOID *)&dev);
    IODestroyPlugInInterface(plug);
    if (hr || !dev) { fprintf(stderr, "QueryInterface failed: %d\n", hr); return 1; }

    kr = (*dev)->USBDeviceOpen(dev);
    if (kr != KERN_SUCCESS) { fprintf(stderr, "USBDeviceOpen failed: %d\n", kr); (*dev)->Release(dev); return 1; }

    printf("Re-enumerating Axe-Fx III USB device...\n");
    kr = (*dev)->USBDeviceReEnumerate(dev, 0);
    (*dev)->USBDeviceClose(dev);
    (*dev)->Release(dev);

    if (kr == KERN_SUCCESS) { printf("ReEnumerate OK — audio should reappear in a few seconds.\n"); return 0; }
    fprintf(stderr, "USBDeviceReEnumerate failed: 0x%08x\n", kr);
    return 1;
}

static int kill_coreaudiod(void) {
    int pids[2048];
    int n = proc_listpids(PROC_ALL_PIDS, 0, pids, sizeof(pids));
    if (n <= 0) { fprintf(stderr, "proc_listpids failed\n"); return 1; }

    int count = n / sizeof(int);
    for (int i = 0; i < count; i++) {
        if (pids[i] == 0) continue;
        char name[64];
        if (proc_name(pids[i], name, sizeof(name)) <= 0) continue;
        if (strcmp(name, "coreaudiod") == 0) {
            if (kill(pids[i], SIGTERM) == 0) {
                printf("coreaudiod (pid %d) killed — launchd will restart it.\n", pids[i]);
                return 0;
            }
            perror("kill coreaudiod");
            fprintf(stderr, "Try running with sudo.\n");
            return 1;
        }
    }
    fprintf(stderr, "coreaudiod not found\n");
    return 1;
}

static void usage(const char *prog) {
    fprintf(stderr,
        "Usage: %s [options]\n"
        "  (no options)          Re-enumerate the Axe-Fx III USB device\n"
        "  -k, --kill-coreaudio  Kill coreaudiod (launchd restarts it automatically)\n"
        "  -a, --all             Do both (USB re-enumerate + kill coreaudiod)\n",
        prog);
}

int main(int argc, char *argv[]) {
    int do_usb = 1, do_kill = 0;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-k") == 0 || strcmp(argv[i], "--kill-coreaudio") == 0) {
            do_usb = 0; do_kill = 1;
        } else if (strcmp(argv[i], "-a") == 0 || strcmp(argv[i], "--all") == 0) {
            do_usb = 1; do_kill = 1;
        } else {
            usage(argv[0]);
            return 1;
        }
    }

    int rc = 0;
    if (do_usb)  rc |= usb_reenumerate();
    if (do_kill) rc |= kill_coreaudiod();
    return rc;
}
