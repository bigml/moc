#ifndef __MPACKET_H__
#define __MPACKET_H__

__BEGIN_DECLS

enum {
    DATA_TYPE_EOF = 0,
    DATA_TYPE_U32,
    DATA_TYPE_ULONG,
    DATA_TYPE_STRING,
    DATA_TYPE_ARRAY,
    DATA_TYPE_ANY                /* used in data_cell_search, include all type */
};

/*
 * unpacket a string into hdf dataset
 * buf: input hdf string
 */
size_t unpack_hdf(unsigned char *buf, size_t len, HDF **hdf);
size_t unpack_data_str(unsigned char *buf, size_t len, char **val);

/*
 * packet a hdf's CHILD to transable string
 * hdf: input hdf dataset, should contain child,
 *      it's own value will be ignore
 */
size_t pack_hdf(HDF *hdf, unsigned char *buf, size_t len);
size_t pack_data_str(const char *key, const char *val,
                     unsigned char *buf, size_t len);

__END_DECLS
#endif    /* __MPACKET_H__ */
