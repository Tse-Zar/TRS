/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = UEFI Bootloader for TRS ---------- =
    ======================================
*/

#include <efi.h>
#include <efilib.h>
#include <bootinfo.h>

#define KERNEL_ADDR 0x100000
static EFI_HANDLE _imagehandle;
static boot_info_t _bootinfo; 

static EFI_GUID img_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static unsigned int popcnt(unsigned v) {
    unsigned int n = 0;
    while(v) {
        n += v & 1;
        v >>= 1;
    }
    return n;
}

static void fatal(const char* msg) {
    Print(u"\r\n<< UEFI FATAL >> %a\r\n", msg);
    while(1) __asm__ volatile("hlt");
}

static UINTN read_kernel(EFI_FILE* root) {
    EFI_FILE* file;

    if(uefi_call_wrapper(root->Open, 5, root, &file, L"\\kernel.bin", EFI_FILE_MODE_READ, 0) != EFI_SUCCESS) {
        fatal("cannot open kernel.");
    }

    UINTN size = 0;
    uefi_call_wrapper(file->SetPosition, 2, file, 0xFFFFFFFFFFFFFFFF);
    uefi_call_wrapper(file->GetPosition, 2, file, &size);
    uefi_call_wrapper(file->SetPosition, 2, file, 0);

    UINTN pages = (size + 0xFFF) / 0x1000 + 64;
    EFI_PHYSICAL_ADDRESS addr = KERNEL_ADDR;
    if(uefi_call_wrapper(BS->AllocatePages, 4,AllocateAddress, EfiLoaderCode, pages, &addr) != EFI_SUCCESS) {
        fatal("Allocate fail.");
    } 
    if(uefi_call_wrapper(file->Read, 3, file, &size, (void*)(UINTN)KERNEL_ADDR) != EFI_SUCCESS) {
        fatal("file read error.");
    }

    uefi_call_wrapper(file->Close, 1, file);

    return size;
}

static void init_framebuf(void) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
    if(uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop) != EFI_SUCCESS || !gop)
        fatal("framebuffer intialization.");

    UINT32 best = gop->Mode->Mode, bestpx = 0;
    for(UINT32 i = 0; i < gop->Mode->MaxMode; ++i) {
        UINTN sz;
        EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* mi = NULL;

        if(uefi_call_wrapper(gop->QueryMode, 4, gop, i, &sz, &mi) != EFI_SUCCESS)
            continue;

        UINT32 px = mi->HorizontalResolution * mi->VerticalResolution;
        if(px > bestpx) {
            bestpx = px;
            best = i;
        }
        uefi_call_wrapper(BS->FreePool, 1, mi);
    }
    if(best != gop->Mode->Mode)
        uefi_call_wrapper(gop->SetMode, 2, gop, best);

    EFI_GRAPHICS_OUTPUT_MODE_INFORMATION* mi = gop->Mode->Info;
    unsigned int rm;
    unsigned int gm;
    unsigned int bm;
    switch (mi->PixelFormat) {
        case PixelRedGreenBlueReserved8BitPerColor: 
            rm = 0xFF0000;
            gm = 0x00FF00;
            bm = 0x0000FF;
            break;
        case PixelBlueGreenRedReserved8BitPerColor:
            rm = 0x0000FF;
            gm = 0x00FF00;
            bm = 0xFF0000;
            break;
        case PixelBitMask:
            rm = mi->PixelInformation.RedMask;
            gm = mi->PixelInformation.GreenMask;
            bm = mi->PixelInformation.BlueMask;
            break;
        default:
            fatal("no linear framebufer");
    }

    unsigned int bits = popcnt(rm) + popcnt(gm) + popcnt(bm);

    _bootinfo.fb = (framebuf_info_t){
        .base   = gop->Mode->FrameBufferBase,
        .size   = gop->Mode->FrameBufferSize,
        .width  = mi->HorizontalResolution,
        .height = mi->VerticalResolution,
        .pitch  = mi->PixelsPerScanLine * 4,
        .bpp    = 32,
        .rmask  = rm, .gmask = gm, .bmask = bm,
        .reserved_mask = mi->PixelInformation.ReservedMask
    };

    if(!_bootinfo.fb.base || !_bootinfo.fb.width || !_bootinfo.fb.height)
        fatal("bad framebuffer.");
}

static void get_mmaped_end_exit(void) {
    UINTN msize = 0x1000;
    UINTN mkey, dsize;
    UINT32 dver;
    EFI_MEMORY_DESCRIPTOR* ds;

    while(1) {
        uefi_call_wrapper(BS->AllocatePool, 3, EfiLoaderData, msize, (void**)&ds);

        EFI_STATUS es = uefi_call_wrapper(BS->GetMemoryMap, 5, &msize, ds, &mkey, &dsize, &dver);
        if(es == EFI_BUFFER_TOO_SMALL) {
            msize += 0x2000;
            continue;
        }
        if(es != EFI_SUCCESS) {
            fatal("memory mapping.");
        }

        _bootinfo.mmap_dsize = dsize;
        _bootinfo.mmap_size  = msize;
        _bootinfo.mmap_addr  = (unsigned long long)(UINTN)ds;

        if(uefi_call_wrapper(BS->ExitBootServices, 2, _imagehandle, mkey) == EFI_SUCCESS) {
            return;
        }

        msize += 0x2000;
    }
}

static void __attribute__((noreturn)) kjump(EFI_PHYSICAL_ADDRESS entry, EFI_PHYSICAL_ADDRESS stack, boot_info_t* bi) { \
    register boot_info_t* rdi_bi __asm__ ("%rdi") = bi;

    __asm__ volatile ( 
        "cli \n\t"
        "movq %0, %%rsp\n\t"
        "xorq %%rbp, %%rbp\n\t" 
        "movq %1, %%rdi\n\t"
        "jmp *%2\n\t"
        : 
        : "r"(stack), "r"(rdi_bi), "r"(entry)
        : "memory"
    ); 
    __builtin_unreachable(); 
}


EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    _imagehandle = ImageHandle;
    /* --- Hello msg --- */
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    Print(u"<< TRS UEFI >>\n");

    EFI_LOADED_IMAGE* li;
    uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &img_guid, (void**)&li);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    uefi_call_wrapper(BS->HandleProtocol, 3, li->DeviceHandle, &sfs_guid, (void**)&fs);
    EFI_FILE* root;
    uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);

    UINTN k_size = read_kernel(root);
    Print(L"Kernel loaded to 0x100000 (%ld bytes)", k_size);

    init_framebuf();

    EFI_PHYSICAL_ADDRESS stack;
    uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 32, &stack);
    _bootinfo.stack_top = stack + 32 * 0x1000;

    get_mmaped_end_exit();

    kjump(KERNEL_ADDR, _bootinfo.stack_top, &_bootinfo);
    return EFI_SUCCESS; 
}