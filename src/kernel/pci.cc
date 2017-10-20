/*
http://wiki.osdev.org/PCI
https://github.com/mallardtheduck/osdev/blob/ffad06176a55722f6c7e220d4c8d07a2fd9b6108/src/modules/pci/pci.cpp
 */
#include "pci.h"
#include "def.h"
#include "asm.h"
#include "array.h"
#include "stdio.h"

constexpr ushort CONFIG_ADDRESS = 0xcf8;
constexpr ushort CONFIG_DATA = 0xcfc;

static uint make_pci_address(uchar bus, uchar device, uchar func, uchar offset) {
    uint lbus = bus;
    uint ldevice = device;
    uint lfunc = func;
    return 0x80000000 | (lbus << 16) | (ldevice << 11) | (func << 8) | offset;
}

static uint read_dword(uchar bus, uchar device, uchar func, uchar offset) {
    auto addr = make_pci_address(bus, device, func, offset & ~3);
    outd(CONFIG_ADDRESS, addr);
    return ind(CONFIG_DATA);
}

static ushort read_word(uchar bus, uchar device, uchar func, uchar offset) {
    uint val = read_dword(bus, device, func, offset & ~3);
    return (val >> ((offset & 2) << 3)) & 0xffff;
}

struct DeviceInfo {
    uchar bus, device, func;
    bool present;
    ushort vendor_id;
    ushort device_id;
    uchar header_type;
    uchar classcode;
    uchar subclass;
    uchar int_line;
    
    void scan(uchar bus, uchar device, uchar func) {
        this->bus = bus;
        this->device = device;
        this->func = func;
        uint id = read_dword(bus, device, func, 0);
        vendor_id = id & 0xffff;
        device_id = id >> 16;
        present = vendor_id != 0xffff;
        if (present) {
            header_type = read_word(bus, device, func, 0x0e) & 0xff;
            ushort cls = read_word(bus, device, func, 0x0a);
            classcode = cls >> 8;
            subclass = cls & 0xff;
            ushort interrupt = read_word(bus, device, func, 0x3c);
            int_line = interrupt & 0xff;
        }
    }
    
    bool is_device() {
        return (header_type & 0x7f) == 0;
    }
    
    bool is_pci_to_pci_bridge() {
        return header_type == 1 && classcode == 0x06 && subclass == 0x04;
    }
    
    void show() {
        printf("Vendor %2x Device %2x Cls %1x Subcls %1x Int %1x\n",
                vendor_id, device_id, classcode, subclass, int_line);
    }
};

Array<DeviceInfo> infos;

DeviceInfo& find_device_by_class(uchar classcode, uchar subclass) {
    for (auto& info: infos) {
        if (info.classcode == classcode && info.subclass == subclass) {
            return info;
        }
    }
}

void init_pci() {
    infos.construct();
    for (uchar device = 0; device < 32; ++device) {
        for (uchar func = 0; func < 8; ++func) {
            DeviceInfo info;
            info.scan(0, device, func);
            if (info.present) {
                if (info.is_device()) {
                    infos.append(info);
                } else if (info.is_pci_to_pci_bridge()) {
                    printf("is_pci_to_pci_bridge\n");
                } else {
                    // not handled
                }
            }
        }
    }
    auto info = find_device_by_class(0x1, 0x1);
    info.show();
    uint bar4 = read_dword(info.bus, info.device, info.func, 0x20);
    printf("%x\n", bar4);
    panic("");
}
