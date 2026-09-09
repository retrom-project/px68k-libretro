#include <assert.h>
#include "../../x68k/disk_dim.c"
#include "../../x68k/disk_xdf.c"
static uint8_t saved[2 * 1024 * 1024];
int PX68KSS_StateAction(void *st, int load, int data_only, SFORMAT *fields, const char *name, bool optional) {
    (void)st; (void)data_only; (void)name; (void)optional;
    size_t offset = 0;
    for (SFORMAT *field = fields; field->name; ++field) {
        assert(offset + field->size <= sizeof(saved));
        if (load) memcpy(field->v, saved + offset, field->size);
        else memcpy(saved + offset, field->v, field->size);
        offset += field->size;
    }
    return 1;
}
int main(void) {
    DIMImg[0] = calloc(1, 1024*9*170+sizeof(DIM_HEADER));
    DIMCur[0] = 7; DIMTrk[0] = 40; DIMImg[0][4000] = 19;
    assert(DIM_StateAction(NULL, 0, 0));
    DIMCur[0] = 0; DIMTrk[0] = 0; DIMImg[0][4000] = 99;
    assert(DIM_StateAction(NULL, 1, 0));
    assert(DIMCur[0] == 7 && DIMTrk[0] == 40 && DIMImg[0][4000] == 19);
    free(DIMImg[0]); DIMImg[0] = NULL;
    XDFImg[0] = calloc(1, 1261568);
    XDFCur[0] = 6; XDFTrk[0] = 80; XDFImg[0][5000] = 23;
    assert(XDF_StateAction(NULL, 0, 0));
    XDFCur[0] = 0; XDFTrk[0] = 0; XDFImg[0][5000] = 88;
    assert(XDF_StateAction(NULL, 1, 0));
    assert(XDFCur[0] == 6 && XDFTrk[0] == 80 && XDFImg[0][5000] == 23);
    free(XDFImg[0]); XDFImg[0] = NULL;
    return 0;
}
