
#define IS_HEX_CHAR(c) (            \
    ((c) >= 'A' && (c) <= 'F') ||   \
    ((c) >= 'a' && (c) <= 'f') ||   \
    ((c) >= '0' && (c) <= '9'))

static int hexchr2val(char c) {
    if (c >= '0' && c <= '9')
        return c - '0';
    else if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    else if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    return 0;
}

int hex2bin(unsigned char *out, char const *in) {
    int i = 0;

    while (1) {
        if (!IS_HEX_CHAR(in[i]) || !IS_HEX_CHAR(in[i+1]))
            return i / 2;

        out[i / 2] = hexchr2val(in[i]) << 4;
        out[i / 2] |= hexchr2val(in[i+1]);

        i += 2;
    }
}