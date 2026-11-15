/*
    ===============================
    = TSEZAR TSEZAR TSEZAR TSEZAR =
    = --------------------------- =
    = Procetction. Safety. ------ =
    ===============================
*/

#ifndef TRS_SECURITY_H
#define TRS_SECURITY_H

/* --- stack guard --- */
void stack_guard_init(void);
__attribute__((noreturn)) void __stack_chk_fail(void);

#endif // TRS_SECURITY_H