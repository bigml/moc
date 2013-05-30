#ifndef __TCP_H__
#define __TCP_H__

__BEGIN_DECLS

int moc_add_tcp_server(moc_t *evt, const char *addr, int port, int nblock, void *tv);
int tcp_srv_send(moc_srv *srv, unsigned char *buf, size_t bsize);
uint32_t tcp_get_rep(moc_srv *srv, unsigned char *buf, size_t bsize,
                     unsigned char **payload, size_t *psize);

__END_DECLS
#endif  /* __TCP_H__ */
