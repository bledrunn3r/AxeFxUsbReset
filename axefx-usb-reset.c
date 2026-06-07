#include <stdio.h>
#include <CoreFoundation/CoreFoundation.h>
#include <IOKit/IOKitLib.h>
#include <IOKit/usb/IOUSBLib.h>
#include <IOKit/IOCFPlugIn.h>

#define VENDOR_ID  9318   /* 0x2466 Fractal Audio */
#define PRODUCT_ID 32784  /* 0x8010 Axe-Fx III   */

int main(void) {
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
