#ifndef DSP_HANDLER
#define DSP_HANDLER

#include <types.h>
#include <ipc.h>

/* --- utils --- */
typedef struct {
    uint magic;
    uint version;
    uint headersize;
    uint flags;
    uint length;
    uint charsize;
    uint height;
    uint width;
} __attribute__((packed)) psf_header;

#define DSP_EP_NAME_MAX 16 // dsp handler endpoints

/* --- message for dsp handler --- */
typedef enum {
    DSP_MSG_NONE = 0,   // none just for memset(..., 0, ...);
    DSP_MSG_PRINT,      // print text
    DSP_MSG_SET_COLOR,  // set foreground/basckgorund colors
    DSP_MSG_SET_CURSOR, // set cursor position
    DSP_MSG_CLEAR,      // clear dsp
    DSP_FLIP,           // flip backbuffer to framebuffer
    DSP_MSG_GET_CURSOR, // get current cursor position
    DSP_MSG_GET_SIZE    // get screen size (why not?)
} dsp_msg_type_t; // messge types

typedef struct {
    dsp_msg_type_t type;        // message type see high
    uint           x;           // screen cord
    uint           y;           // ^^^^
    uint           fg_color;    // foreground color
    uint           bg_colot;    // background color
    word_t         text_ptr;    // pointer to text
    uint           text_len;    // length of text
    word_t         reserved;    // aligment =( maybe there is something useful
} __attribute__((packed)) dsp_msg_t; // 8 words structure (for fastpath)

/* --- dsp handler state --- */
typedef struct {
    ipc_cap_t ep;
    thread_t* handler_th;
    uint      width;
    uint      height;
    uint      cursor_x;
    uint      cursor_y;
    uint      fg_color;
    uint      bg_color;
    int       ready;
} dsp_handler_t;

/* --- API --- */
int dsp_handler_init(void);
void dsp_handler_destroy(void);

/* --- IPC calls --- */
int dsp_handler_send_text(const char* txt, uint len, uint x, uint y);
int dsp_handler_set_color(uint fg, uint bg);
int dsp_handler_set_cursor(uint x, uint y);
int dsp_handler_clear(void);
int dsp_hanldler_flip(void);

void dsp_handler_callback(dsp_msg_t* msg, ipc_err_t result);


#endif // DSP_HANDLER
