/*
    ======================================
    = TSEZAR TSEZAR TSEZAR TSEZAR TSEZAR =
    = ---------------------------------- =
    = UEFI Bootloader for TRS ---------- =
    = 4K page bitmap with TRS class ---- =
    = ----------- (unused/code/data/dev) =
    ======================================
*/

#include <efi.h>
#include <efilib.h>
#include <bootinfo.h>

#define EFER_MSR 0xC0000080ULL
#define EFER_NXE (1ULL << 11)
#define PG       0x1000ULL

#define KERNEL_PATH    L"\\space\\sys\\kern\\kernel.bin"
#define KERN_BSS_SLACK 256
#define LOW_TOP        0x100000000ULL
#define MAP_SLACK      0x8000
#define MMIO_SLACK     0x10000000ULL

typedef struct {
    unsigned long long e[512];
} __attribute__((aligned(4096))) pt_t;

static EFI_HANDLE    _imagehandle;
static boot_info_t   _bootinfo;
static pt_t*         _pml4;

static EFI_GUID img_guid = EFI_LOADED_IMAGE_PROTOCOL_GUID;
static EFI_GUID sfs_guid = EFI_SIMPLE_FILE_SYSTEM_PROTOCOL_GUID;
static EFI_GUID gop_guid = EFI_GRAPHICS_OUTPUT_PROTOCOL_GUID;

static unsigned char*     _bmap;
static unsigned long long _bmap_pages, _bmap_phys;
static unsigned long long _total_pages, _ram_top;

static EFI_MEMORY_DESCRIPTOR* _mmap;
static unsigned long long _mmap_size, _mmap_dsize, _mmap_key, _mmap_phys, _mmap_buf_pages;
static unsigned long long _kern_phys, _kern_pages, _kern_code_pages, _kern_entry, _kern_size;
static unsigned long long _stack_phys, _stack_top;
static unsigned long long _bi_phys, _img_base, _img_pages;


#define DESC_AT(base, dsize, i) \
    ((EFI_MEMORY_DESCRIPTOR*)((char*)(base) + (unsigned long long)(i) * (unsigned long long )(dsize)))

static void fatal(const char* msg) {
    Print(L"\r\n<< UEFI FATAL >> %a\r\n", msg);
    while(1) __asm__ volatile("hlt");
}

static unsigned long long pages_of(unsigned long long bytes) {
    return (bytes + PG - 1) / PG;
}

void* b_memcpy(void* dst, const void* src, size_t len) {
    unsigned char* d = (unsigned char*)dst;
    const unsigned char* s = (const unsigned char*)src;

    while(len > 0 && ((uintptr_t)d & 3) != 0) {
        *d++ = *s++;
        --len;
    }

    int s_is_aligned = ((uintptr_t)s & 3) == 0;

    uint32_t* d32 = (uint32_t*)d;
    uint32_t* s32 = (uint32_t*)s;

    while(len >= 32 && s_is_aligned) {
        d32[0] = s32[0];
        d32[1] = s32[1];
        d32[2] = s32[2];
        d32[3] = s32[3];
        d32[4] = s32[4];
        d32[5] = s32[5];
        d32[6] = s32[6];
        d32[7] = s32[7];

        d32 += 8;
        s32 += 8;
        len -= 32;
    }

    while(len >= 4 && s_is_aligned) {
        *d32++ = *s32++;
        len -= 4;
    }

    d = (unsigned char*)d32;
    s = (unsigned char*)s32;

    while(len > 0) {
        *d++ = *s++;
        len--;
    }
    return dst;
}
static unsigned long long alloc_pages(unsigned long long max_addr, unsigned long long n, UINT32 type) {
    unsigned long long a = max_addr ? max_addr - 1 : 0;

    if(!max_addr || uefi_call_wrapper(BS->AllocatePages, 4, AllocateMaxAddress, type, n, &a) != EFI_SUCCESS) {
        a = 0;
        if(uefi_call_wrapper(BS->AllocatePages, 4, AllocateAnyPages, type, n, &a) != EFI_SUCCESS)
            fatal("out of memory");
    }

    uefi_call_wrapper(BS->SetMem, 3, (void*)(UINTN)a, n * 0x1000, 0);
    return a;
}

static pt_t* sub_table(unsigned long long* e) {
    if(*e & PTE_P) return (pt_t*)(UINTN)(*e & PTE_ADDR);

    unsigned long long p = alloc_pages(LOW_TOP, 1, EfiLoaderData);
    *e = p | PTE_P | PTE_WR;
   
    return (pt_t*)(UINTN)p;
}

static void map_4k(unsigned long long vaddr, unsigned long long paddr, unsigned long long bytes, 
                   unsigned long long flags) {
    for(unsigned long long i = 0; i < bytes; i += PG) {
        unsigned long long v = vaddr + i;
        pt_t* pdpt = sub_table(&_pml4->e[(v >> 39) & 511]);
        pt_t* pd   = sub_table(&pdpt->e[(v >> 30) & 511]);
        pt_t* pt   = sub_table(&pd->e[(v >> 21) & 511]);
        
        pt->e[(v >> 12) & 511] = ((paddr + i) & PTE_ADDR) | flags | PTE_P;
    }
}

static void enable_nx(void) {
    unsigned int a, b, c, d;

    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000000U));
    if(a < 0x80000001U)
        return;

    __asm__ volatile("cpuid" : "=a"(a), "=b"(b), "=c"(c), "=d"(d) : "a"(0x80000001U));
    if(!(d & (1U << 20)))
        return;

    __asm__ volatile("rdmsr" : "=a"(a), "=d"(d) : "c"((unsigned int)EFER_MSR));
    a |= (unsigned int)EFER_NXE;
    __asm__ volatile("wrmsr" :: "c"((unsigned int)EFER_MSR), "a"(a), "d"(d));
}

static int id_class(UINT32 type, unsigned long long* flags) {
    switch (type) {
        case EfiConventionalMemory:
        case EfiLoaderData:
        case EfiLoaderCode:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiACPIReclaimMemory:
            *flags = MM_UNUSED;
            return 1;
        
        case EfiRuntimeServicesData:
            *flags = MM_DATA;
            return 1;
        case EfiRuntimeServicesCode:
            *flags = MM_CODE;
            return 1;
        case EfiMemoryMappedIO:
        case EfiMemoryMappedIOPortSpace:
            *flags = MM_DEV;
            return 1;
        default:
            *flags = 0;
            return 0;
    }
}

static int is_ram(UINT32 type) {
    switch (type) {
        case EfiConventionalMemory:
        case EfiLoaderCode:
        case EfiBootServicesCode:
        case EfiBootServicesData:
        case EfiRuntimeServicesCode:
        case EfiRuntimeServicesData:
        case EfiACPIReclaimMemory:
        case EfiACPIMemoryNVS:
        case EfiLoaderData:
        case EfiPersistentMemory:
            return 1;
        default:
            return 0;
    }
}

static int is_free(UINT32 type) {
    return type == EfiConventionalMemory || type == EfiLoaderCode       || type == EfiLoaderData || 
           type == EfiBootServicesCode   || type == EfiBootServicesData || type == EfiACPIReclaimMemory;
}

static EFI_STATUS read_map(void* buf, unsigned long long cap, UINTN* size, UINTN* key, 
                           UINTN* dsize) {
    UINTN sz = (UINTN)cap;
    UINTN k = 0;
    UINTN d = 0;

    UINT32 ver = 0;
    EFI_STATUS rc = uefi_call_wrapper(BS->GetMemoryMap, 5, &sz, buf, &k, &d, &ver);
    *size = sz;
    if(rc == EFI_SUCCESS) {
        *key = k;
        *dsize = d;
    }

    return rc;
}

static EFI_MEMORY_DESCRIPTOR* probe_map(unsigned long long* size, unsigned long long* dsize) {
    UINTN sz  = 0;
    UINTN key = 0;
    UINTN d   = 0;
    read_map(NULL, 0, &sz, &key, &d);
    sz += MAP_SLACK;

    EFI_MEMORY_DESCRIPTOR* desc = (EFI_MEMORY_DESCRIPTOR*)(UINTN)
                               alloc_pages(LOW_TOP, pages_of(sz), EfiLoaderData);

    if(read_map(desc, sz, &sz, &key, &d) != EFI_SUCCESS) 
        fatal("Get MemoryMap.");

    *size  = sz;
    *dsize = d;
    return desc;
}

static void build_identity(EFI_MEMORY_DESCRIPTOR* desc, unsigned long long size, unsigned long long dsize){
    unsigned long long limit = _ram_top + MMIO_SLACK;

    for(unsigned long long i = 0; i * dsize < size; ++i) {
        EFI_MEMORY_DESCRIPTOR* d = DESC_AT(desc, dsize, i);

        unsigned long long flags;
        if(!id_class(d->Type, &flags)) continue;
        if(d->PhysicalStart >= limit) continue;

        map_4k(d->PhysicalStart, d->PhysicalStart, d->NumberOfPages * PG, flags);
    }
}

static void bm_set(unsigned long long p) {
    if (p < _total_pages) _bmap[p >> 3] |= (1U << (p & 7)); 
}
static void bm_clr(unsigned long long p) {
    if(p < _total_pages) _bmap[p >> 3] &= ~(1U << (p & 7));
}
static void bm_used(unsigned long long paddr, unsigned long long n) {
    for(unsigned long long i = 0; i < n; ++i) {
        bm_set(paddr / PG + i);
    }
}

static void mark_tables(pt_t* t, int lvl) {
    bm_used((unsigned long long)(UINTN)t, 1);
    if(lvl <= 2) return;
    for(int i = 0; i < 512; ++i) {
        if((t->e[i] & PTE_P) && !(t->e[i] & PTE_PS))
            mark_tables((pt_t*)(UINTN)(t->e[i] & PTE_ADDR), lvl - 1);
    }
}

static void build_bitmap(void) {
    for(unsigned long long i = 0; i < _bmap_pages * PG; ++i) {
        _bmap[i] = 0xFF;
    }

    for(unsigned long long i = 0; i * _mmap_dsize < _mmap_size; ++i) {
        EFI_MEMORY_DESCRIPTOR* d = DESC_AT(_mmap, _mmap_dsize, i);
        if(!is_free(d->Type)) continue;
        for(unsigned long long p = 0; p < d->NumberOfPages; ++p) {
            bm_clr(d->PhysicalStart / PG + p);
        }
    }

    bm_used(0, 16);
    bm_used(_kern_phys, _kern_pages);
    bm_used(_stack_phys, KERNEL_STACK_PG);
    bm_used(_bi_phys, 1);
    bm_used(_bmap_phys, _bmap_pages);
    bm_used(_mmap_phys, _mmap_buf_pages);
    bm_used(_img_base, _img_pages);
    mark_tables(_pml4, 4);
}

static void load_kernel(EFI_FILE* root) {
    EFI_FILE* file;
    if(uefi_call_wrapper(root->Open, 5, root, &file, KERNEL_PATH, EFI_FILE_MODE_READ, 0) != EFI_SUCCESS)
        fatal("cannot open kernel.");

    UINTN size = 0;
    uefi_call_wrapper(file->SetPosition, 2, file, 0xFFFFFFFFFFFFFFFFULL);
    uefi_call_wrapper(file->GetPosition, 2, file, &size);
    uefi_call_wrapper(file->SetPosition, 2, file, 0);

    _kern_size  = size;
    _kern_pages = pages_of(size) + KERN_BSS_SLACK; 
    _kern_phys = alloc_pages(LOW_TOP, _kern_pages, EfiLoaderCode);
    for(UINTN off = 0; off < size;) {
        UINTN n = size - off;
        if(uefi_call_wrapper(file->Read, 3, file, &n, (void*)(UINTN)_kern_phys + off) != EFI_SUCCESS)
            fatal("file read error.");
        if(!n) break;
        off += n;
    }
    uefi_call_wrapper(file->Close, 1, file);

    kern_hdr_t* hdr = (kern_hdr_t*)(UINTN)_kern_phys;
    if(hdr->magic == KERN_MAGIC) {
        unsigned long long need = hdr->mem_pages ? hdr->mem_pages : pages_of(hdr->image_size);
        if(need > _kern_pages)
            fatal("kernel need more memory.");
        _kern_pages      = need;
        _kern_code_pages = hdr->text_pages;
        _kern_entry      = KERNEL_VBASE + hdr->entry_off;
    } else {
        _kern_code_pages = pages_of(size) - 1;
        _kern_entry      = KERNEL_VBASE;
        Print(L"[BOOT] no kern header: code = %ld data 1+%ld\r\n", (unsigned long long)_kern_code_pages,
              (unsigned long long)KERN_BSS_SLACK);
    }

    if(_kern_code_pages >= _kern_pages) _kern_code_pages = _kern_pages - 1;
}

static void map_kernel(void) {
    /* --- kernel data ---*/
    unsigned long long phys  = _kern_phys + _kern_code_pages * PG;
    unsigned long long virt  = KERNEL_VBASE + _kern_code_pages * PG;
    unsigned long long bytes = (_kern_pages - _kern_code_pages) * PG;

    map_4k(_kern_phys, _kern_phys, _kern_code_pages * PG, MM_CODE);
    map_4k(virt, phys, bytes, MM_DATA);
    map_4k(KERNEL_VBASE, _kern_phys, _kern_code_pages * PG, MM_CODE);
    map_4k(KSTACK_VBASE, _stack_phys, KERNEL_STACK_PG * PG, MM_DATA);
    map_4k(BOOTINFO_VBASE, _bi_phys, PG, MM_DATA);
    map_4k(MMAP_VBASE, _mmap_phys, _mmap_buf_pages * PG, MM_DATA);
    map_4k(PMMBMP_VBASE, _bmap_phys, _bmap_pages * PG, MM_DATA);
    map_4k(KERNEL_FB_VBASE, _bootinfo.fb.base, pages_of(_bootinfo.fb.size) * PG, MM_DEV);
}

static void keep_loader_mapped(EFI_MEMORY_DESCRIPTOR* probe, unsigned long long size,
                               unsigned long long dsize) {
    unsigned long long base  = _img_base & ~0xFFFULL;
    unsigned long long pages = _img_pages;
      if(!pages) {
        for(unsigned long long i = 0; i * dsize < size; ++i) {
            EFI_MEMORY_DESCRIPTOR* d = DESC_AT(probe, dsize, i);
            unsigned long long end   = d->PhysicalStart + d->NumberOfPages * PG;

            if(_img_base >= d->PhysicalStart && _img_base < end) {
                base  = d->PhysicalStart & ~0xFFFULL;
                pages = d->NumberOfPages;
                break;
            }
        }
    }

    if(!pages) pages = 0x100;                        /* 1 MiB last resort */
    map_4k(base, base, pages * PG, PTE_P | PTE_WR);
}

static void init_framebuf(void) {
    EFI_GRAPHICS_OUTPUT_PROTOCOL* gop = NULL;
    if(uefi_call_wrapper(BS->LocateProtocol, 3, &gop_guid, NULL, (void**)&gop) != EFI_SUCCESS || !gop)
        fatal("no graphics output protocol.");

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
            rm = 0;
            gm = 0;
            bm = 0;
            fatal("no linear framebufer");
    }

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

static void exit_boot(void) {
    UINTN key  = 0;
    UINTN size = 0;
    UINTN desc = 0;

    for(int retry = 0; retry < 7; ++retry) {
        if(read_map(_mmap, _mmap_buf_pages * PG, &size, &key, &desc) != EFI_SUCCESS)
            continue;
        if(size > _mmap_buf_pages * PG) 
            fatal("mmap buffer too small.");
        
        _mmap_size  = size;
        _mmap_dsize = desc;

        if(uefi_call_wrapper(BS->ExitBootServices, 2, _imagehandle, key) == EFI_SUCCESS)
            return;
    }

    fatal("boot exit error.");
}

extern void tramp(unsigned long long pml4, unsigned long long entry, 
                                  unsigned long long bi_vaddr, unsigned long long kern_paddr, 
                                  unsigned long long stack_top);

EFI_STATUS
EFIAPI
efi_main(EFI_HANDLE ImageHandle, EFI_SYSTEM_TABLE* SystemTable) {
    InitializeLib(ImageHandle, SystemTable);
    _imagehandle = ImageHandle;

    /* --- Hello msg --- */
    uefi_call_wrapper(SystemTable->ConOut->ClearScreen, 1, SystemTable->ConOut);
    Print(u"<< TRS UEFI >>\n");

    EFI_LOADED_IMAGE* li;
    if(uefi_call_wrapper(BS->HandleProtocol, 3, ImageHandle, &img_guid, (void**)&li) != EFI_SUCCESS)
        fatal("loaded image protocol");

    _img_base  = (unsigned long long)li->ImageBase;
    _img_pages = pages_of(li->ImageSize);

    EFI_SIMPLE_FILE_SYSTEM_PROTOCOL* fs;
    if(uefi_call_wrapper(BS->HandleProtocol, 3, li->DeviceHandle, &sfs_guid, (void**)&fs) != EFI_SUCCESS)
        fatal("file system protocol");

    EFI_FILE* root;
    if(uefi_call_wrapper(fs->OpenVolume, 2, fs, &root) != EFI_SUCCESS)
        fatal("open volume");

    unsigned long long size, desc_size;
    EFI_MEMORY_DESCRIPTOR* probe = probe_map(&size, &desc_size);
    
    _ram_top = 0;
    for(unsigned long long i = 0; i * desc_size < size; ++i) {
        EFI_MEMORY_DESCRIPTOR* d = DESC_AT(probe, desc_size, i);
        if(!is_ram(d->Type))
            continue;

        unsigned long long end = d->PhysicalStart + d->NumberOfPages * PG;
        if(end > _ram_top) 
            _ram_top = end;
    }
    _ram_top     = (_ram_top + 0x1FFFFFULL) & ~0x1FFFFFULL;
    _total_pages = _ram_top / PG;
    _bmap_pages  = pages_of((_total_pages + 7) >> 3);
    Print(L"[BOOT]: ram top %x, %x pages, bitmap %x pages\r\n", _ram_top, _total_pages, _bmap_pages);

    load_kernel(root);
    Print(L"[BOOT]: kernel phys addr: %x, %x pages (code %x), entry %x\r\n", _kern_phys, _kern_pages,
          _kern_code_pages, _kern_entry);
    
    _stack_phys = alloc_pages(LOW_TOP, KERNEL_STACK_PG, EfiLoaderData);
    _stack_top  = KSTACK_VBASE + KERNEL_STACK_PG * PG;
    _bi_phys    = alloc_pages(LOW_TOP, 1, EfiLoaderData);
    _bmap_phys  = alloc_pages(LOW_TOP, _bmap_pages, EfiLoaderData);
    _bmap       = (unsigned char*)(UINTN)_bmap_phys;

    _mmap_buf_pages = pages_of(size + MAP_SLACK);
    _mmap_phys      = alloc_pages(LOW_TOP, _mmap_buf_pages, EfiLoaderData);
    _mmap           = (EFI_MEMORY_DESCRIPTOR*)(UINTN)_mmap_phys;

    _pml4 = (pt_t*)(UINTN)alloc_pages(LOW_TOP, 1, EfiLoaderData);
    build_identity(probe, size, desc_size);
    keep_loader_mapped(probe, size, desc_size);

    init_framebuf();
    map_kernel();

    _bootinfo.mmap_vaddr    = MMAP_VBASE;
    _bootinfo.stack_top     = _stack_top;
    _bootinfo.identity_base = 0;
    _bootinfo.kern_base     = KERNEL_VBASE;
    _bootinfo.kern_size     = _kern_size;
    _bootinfo.hight_half    = 1;
    _bootinfo.bitmap_vaddr  = PMMBMP_VBASE;
    _bootinfo.bitmap_size   = _bmap_pages * PG;
    _bootinfo.mmap_paddr    = _mmap_phys;
    _bootinfo.kern_paddr    = _kern_phys;
    _bootinfo.kern_code_pg  = _kern_code_pages;
    _bootinfo.fb.base       = KERNEL_FB_VBASE;
    
    exit_boot();
    build_bitmap();
    _bootinfo.mmap_size   = _mmap_size;
    _bootinfo.mmap_dsize  = _mmap_dsize;
    _bootinfo.total_pages = _total_pages;
    b_memcpy((void*)(UINTN)_bi_phys, &_bootinfo, sizeof(_bootinfo));
    
    enable_nx();
    
    tramp((unsigned long long)(UINTN)_pml4, _kern_entry, _bi_phys, _kern_phys, _stack_top);
    return EFI_SUCCESS; 
}