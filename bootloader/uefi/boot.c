#include <efi.h>
#include <efilib.h>
#include <bootinfo.h>

typedef void (*kernel_entry_t)(boot_info_t* bi);

#define KERNEL_ADDR 0x100000
static EFI_HANDLE _imagehandle;
static boot_info_t _bootinfo; 

static EFI_GUID img_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static void fatal(const char* msg) {
    Print(u"\r\n<< UEFI FATAL >> %s\r\n", msg);
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
    if(uefi_call_wrapper(BS->AllocatePages, 4,AllocateAddress, EfiLoaderData, pages, &addr) != EFI_SUCCESS) {
        fatal("AllocatePool fail.");
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

   _bootinfo.framebuf_addr   = gop->Mode->FrameBufferBase;
   _bootinfo.framebuf_size   = gop->Mode->FrameBufferSize;
   _bootinfo.framebuf_height = gop->Mode->Info->VerticalResolution;
   _bootinfo.framebuf_width  = gop->Mode->Info->HorizontalResolution;
   _bootinfo.framebuf_bpp    = gop->Mode->Info->PixelFormat;
   _bootinfo.framebuf_pitch  = gop->Mode->Info->PixelsPerScanLine * 4;
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

static void __attribute__((noreturn)) kjump(EFI_PHYSICAL_ADDRESS entry, EFI_PHYSICAL_ADDRESS stack, boot_info_t* bi) { 
    __asm__ volatile ( 
        "cli \n\t"
        "movq %0, %%rsp\n\t"
        "xorq %%rbp, %%rbp\n\t" 
        "movq %1, %%rdi\n\t"
        "jmp *%2\n\t"
        : 
        : "r"(stack), "r"(bi), "r"(entry)
        : "memory"
    ); 
    __builtin_unreachable(); 
}


EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);

    /* --- Hello msg --- */
    SystemTable->ConOut->ClearScreen(SystemTable->ConOut);
    Print(u"<< TRS UEFI >>\n");

    EFI_LOADED_IMAGE* li;
    uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &img_guid, (void**)&li);
    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    uefi_call_wrapper(BS->HandleProtocol, 3, li->DeviceHandle, &sfs_guid, (void**)&fs);
    EFI_FILE* root;
    uefi_call_wrapper(fs->OpenVolume, 2, fs, &root);

    UINTN k_size = read_kernel(root);
    Print(L"Kernel loaded to 0x100000 (%d bytes)", k_size);

    init_framebuf();

    EFI_PHYSICAL_ADDRESS stack;
    uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, EfiLoaderData, 32, &stack);
    _bootinfo.stack_top = stack + 32 * 0x1000;

    get_mmaped_end_exit();

    kjump(KERNEL_ADDR, _bootinfo.stack_top, &_bootinfo);
    return EFI_SUCCESS; 
}