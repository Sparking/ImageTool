#pragma once

enum {
    QR_MODE_TERMINATOR              = 0x00, /* 终止符 */
    QR_MODE_NUMBERIC                = 0x01, /* 数字 */
    QR_MODE_ALPHANUMBERIC           = 0x02, /* 字母数字 */
    QR_MODE_STRUCTURED_APPEND       = 0x03, /* 结构链接, 不支持 */
    QR_MODE_BYTE                    = 0x04, /* 8位字节 */
    QR_MODE_ECI_MODE                = 0x07, /* ECI */
    QR_MODE_FNC1_FIRST_POSITION     = 0x05, /* FNC1 第一位置 */
    QR_MODE_FNC1_SECOND_POSITION    = 0x09, /* FNC1 第二位置 */
    QR_MODE_HANZI                   = 0x0D, /* GBT 18284-2000, 中国汉字 */
};

#pragma pack(push, 1)
struct qr_bitstream_decode {
    struct qr_bitstream_decode *next;
    unsigned int size;
    unsigned int eci_task_id;
    unsigned char is_eci;
    unsigned char mode;
    unsigned char submode;
    unsigned char data[0];
};
#pragma pack(pop)

extern struct qr_bitstream_decode *qr_bitstream_decode(const unsigned char *bytes,
    const unsigned int bytes_length, const unsigned char version);
